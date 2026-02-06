"""
EtherDream DAC output driver for BeamCommander
Implements communication with EtherDream laser DAC hardware
"""
import socket
import struct
import logging
import threading
import time
from typing import List, Tuple, Optional

logger = logging.getLogger(__name__)


class EtherDreamDAC:
    """
    EtherDream DAC driver for laser output
    
    Protocol based on EtherDream specifications:
    - Discovery via UDP broadcast on port 7654
    - Command/data via TCP on port 7765
    - Point format: X, Y, R, G, B, I (intensity), U (user data), flags
    """
    
    # EtherDream protocol constants
    BROADCAST_PORT = 7654
    COMMAND_PORT = 7765
    
    # Command bytes
    CMD_PREPARE_STREAM = b'p'
    CMD_BEGIN_STREAM = b'b'
    CMD_POINT_RATE = b'q'
    CMD_DATA = b'd'
    CMD_STOP = b's'
    CMD_PING = b'?'
    
    # Point rate (points per second)
    DEFAULT_PPS = 30000
    
    def __init__(self, dac_ip: Optional[str] = None, pps: int = DEFAULT_PPS):
        """
        Initialize EtherDream DAC connection
        
        Args:
            dac_ip: IP address of DAC (None = auto-discover)
            pps: Points per second output rate
        """
        self.dac_ip = dac_ip
        self.pps = pps
        self.sock: Optional[socket.socket] = None
        self.connected = False
        self.streaming = False
        self._lock = threading.RLock()
        
        logger.info(f"Initializing EtherDream DAC (PPS: {pps})")
        
        # Try to discover and connect
        if not self.dac_ip:
            self.dac_ip = self.discover()
        
        if self.dac_ip:
            self.connect()
    
    def discover(self, timeout: float = 2.0) -> Optional[str]:
        """
        Discover EtherDream DAC on network via broadcast
        
        Args:
            timeout: Discovery timeout in seconds
            
        Returns:
            IP address of first discovered DAC, or None
        """
        logger.info("Discovering EtherDream DAC...")
        
        try:
            # Create UDP socket for broadcast listening
            udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            udp_sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
            udp_sock.settimeout(timeout)
            udp_sock.bind(('', self.BROADCAST_PORT))
            
            # Wait for broadcast message from DAC
            try:
                data, addr = udp_sock.recvfrom(1024)
                dac_ip = addr[0]
                logger.info(f"Discovered EtherDream DAC at {dac_ip}")
                udp_sock.close()
                return dac_ip
            except socket.timeout:
                logger.warning("No EtherDream DAC found on network")
                udp_sock.close()
                return None
        except Exception as e:
            logger.error(f"Error during DAC discovery: {e}")
            return None
    
    def connect(self) -> bool:
        """
        Connect to EtherDream DAC via TCP
        
        Returns:
            True if connected successfully
        """
        if not self.dac_ip:
            logger.error("No DAC IP address available")
            return False
        
        try:
            with self._lock:
                # Close existing connection if any
                if self.sock:
                    try:
                        self.sock.close()
                    except:
                        pass
                
                # Create TCP connection
                logger.info(f"Connecting to EtherDream DAC at {self.dac_ip}:{self.COMMAND_PORT}")
                self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.sock.settimeout(5.0)
                self.sock.connect((self.dac_ip, self.COMMAND_PORT))
                
                # Wait for initial status message
                status = self.sock.recv(20)
                if len(status) >= 18:
                    self.connected = True
                    logger.info("Connected to EtherDream DAC")
                    
                    # Prepare for streaming
                    self.prepare_stream()
                    return True
                else:
                    logger.error("Invalid status response from DAC")
                    return False
        except Exception as e:
            logger.error(f"Error connecting to DAC: {e}")
            self.connected = False
            return False
    
    def prepare_stream(self):
        """Prepare DAC for streaming"""
        if not self.connected or not self.sock:
            return
        
        try:
            with self._lock:
                # Send prepare command
                self.sock.send(self.CMD_PREPARE_STREAM)
                
                # Set point rate
                rate_cmd = self.CMD_POINT_RATE + struct.pack('<I', self.pps)
                self.sock.send(rate_cmd)
                
                # Begin streaming
                self.sock.send(self.CMD_BEGIN_STREAM)
                self.streaming = True
                
                logger.info("DAC prepared for streaming")
        except Exception as e:
            logger.error(f"Error preparing stream: {e}")
            self.streaming = False
    
    def send_points(self, points: List[Tuple[float, float, int, int, int]]) -> bool:
        """
        Send point data to DAC
        
        Args:
            points: List of (x, y, r, g, b) tuples
                   x, y in range [-1..1] (will be converted to DAC coords)
                   r, g, b in range [0..255]
        
        Returns:
            True if sent successfully
        """
        if not self.connected or not self.streaming or not self.sock:
            return False
        
        if not points or len(points) == 0:
            return False
        
        try:
            with self._lock:
                # Convert points to EtherDream format
                dac_points = []
                for x, y, r, g, b in points:
                    # Convert from normalized [-1..1] to DAC coords [0..65535]
                    # Center at 32768
                    dac_x = int((x + 1.0) * 32767.5)
                    dac_y = int((y + 1.0) * 32767.5)
                    
                    # Clamp to valid range
                    dac_x = max(0, min(65535, dac_x))
                    dac_y = max(0, min(65535, dac_y))
                    
                    # Convert RGB [0..255] to DAC format [0..65535]
                    dac_r = int(r * 257)  # 255 * 257 = 65535
                    dac_g = int(g * 257)
                    dac_b = int(b * 257)
                    
                    # Intensity (use max of RGB)
                    dac_i = max(dac_r, dac_g, dac_b)
                    
                    # Pack point: x(2), y(2), r(2), g(2), b(2), i(2), u(2), flags(2)
                    point_data = struct.pack('<HHHHHHHxxH',
                                           dac_x, dac_y,
                                           dac_r, dac_g, dac_b,
                                           dac_i,
                                           0,  # user data
                                           0)  # flags
                    dac_points.append(point_data)
                
                # Build data command
                num_points = len(dac_points)
                data_header = self.CMD_DATA + struct.pack('<H', num_points)
                data_payload = b''.join(dac_points)
                
                # Send to DAC
                self.sock.send(data_header + data_payload)
                
                return True
        except Exception as e:
            logger.error(f"Error sending points to DAC: {e}")
            self.connected = False
            self.streaming = False
            return False
    
    def stop(self):
        """Stop streaming and close connection"""
        logger.info("Stopping EtherDream DAC")
        
        with self._lock:
            try:
                if self.sock and self.streaming:
                    self.sock.send(self.CMD_STOP)
                    self.streaming = False
            except:
                pass
            
            try:
                if self.sock:
                    self.sock.close()
                    self.sock = None
            except:
                pass
            
            self.connected = False
    
    def __del__(self):
        """Cleanup on deletion"""
        self.stop()


class DACOutputManager:
    """
    Manages DAC output with automatic reconnection and error handling
    """
    
    def __init__(self, dac_ip: Optional[str] = None, pps: int = 30000, enable: bool = True):
        """
        Initialize DAC output manager
        
        Args:
            dac_ip: IP address of DAC (None = auto-discover)
            pps: Points per second
            enable: Whether to enable DAC output
        """
        self.dac_ip = dac_ip
        self.pps = pps
        self.enabled = enable
        self.dac: Optional[EtherDreamDAC] = None
        self._lock = threading.RLock()
        
        if self.enabled:
            self.connect()
    
    def connect(self) -> bool:
        """Connect to DAC"""
        with self._lock:
            try:
                if self.dac:
                    self.dac.stop()
                
                self.dac = EtherDreamDAC(self.dac_ip, self.pps)
                return self.dac.connected
            except Exception as e:
                logger.error(f"Error creating DAC connection: {e}")
                return False
    
    def send_frame(self, points: List[Tuple[float, float, int, int, int]]) -> bool:
        """
        Send a frame of points to DAC
        
        Args:
            points: List of (x, y, r, g, b) tuples
        
        Returns:
            True if sent successfully
        """
        if not self.enabled:
            return False
        
        with self._lock:
            # Reconnect if not connected
            if not self.dac or not self.dac.connected:
                if not self.connect():
                    return False
            
            # Send points
            success = self.dac.send_points(points)
            
            # Try to reconnect on failure
            if not success and self.enabled:
                logger.warning("DAC send failed, attempting reconnect...")
                time.sleep(0.1)
                self.connect()
            
            return success
    
    def stop(self):
        """Stop DAC output"""
        with self._lock:
            if self.dac:
                self.dac.stop()
                self.dac = None
    
    def set_enabled(self, enabled: bool):
        """Enable or disable DAC output"""
        with self._lock:
            self.enabled = enabled
            if not enabled and self.dac:
                self.dac.stop()
                self.dac = None
            elif enabled and not self.dac:
                self.connect()
