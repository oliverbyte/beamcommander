// BeamCommander Web Interface JavaScript

class BeamCommanderUI {
    constructor() {
        this.canvas = document.getElementById('laser-canvas');
        this.ctx = this.canvas.getContext('2d');
        this.apiBase = '';
        this.updateInterval = 50; // 20 FPS
        this.running = true;
        
        this.setupEventListeners();
        this.startAnimation();
        this.updateStatus();
    }
    
    setupEventListeners() {
        // Shape buttons
        document.querySelectorAll('.shape-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                const shape = e.target.dataset.shape;
                this.sendOSC('/laser/shape', [shape]);
                this.setActiveButton('.shape-btn', e.target);
            });
        });
        
        // Color buttons
        document.querySelectorAll('.color-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                const color = e.target.dataset.color;
                this.sendOSC('/laser/color', [color]);
                this.setActiveButton('.color-btn', e.target);
            });
        });
        
        // Movement buttons
        document.querySelectorAll('.move-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                const move = e.target.dataset.move;
                this.sendOSC('/move/mode', [move]);
                this.setActiveButton('.move-btn', e.target);
            });
        });
        
        // Brightness slider
        document.getElementById('brightness').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('brightness-val').textContent = e.target.value + '%';
            this.sendOSC('/laser/brightness', [value]);
        });
        
        // Dot amount slider
        document.getElementById('dot-amount').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('dot-val').textContent = e.target.value + '%';
            this.sendOSC('/laser/dotted', [value]);
        });
        
        // Rainbow amount
        document.getElementById('rainbow-amount').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('rainbow-amount-val').textContent = e.target.value + '%';
            this.sendOSC('/laser/rainbow/amount', [value]);
        });
        
        // Rainbow speed
        document.getElementById('rainbow-speed').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('rainbow-speed-val').textContent = value.toFixed(2);
            this.sendOSC('/laser/rainbow/speed', [value]);
        });
        
        // Movement size
        document.getElementById('move-size').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('move-size-val').textContent = e.target.value + '%';
            this.sendOSC('/move/size', [value]);
        });
        
        // Movement speed
        document.getElementById('move-speed').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('move-speed-val').textContent = value.toFixed(2);
            this.sendOSC('/move/speed', [value]);
        });
        
        // Scale
        document.getElementById('scale').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('scale-val').textContent = value.toFixed(2);
            this.sendOSC('/laser/shape/scale', [value]);
        });
        
        // Rotation speed
        document.getElementById('rotation-speed').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('rotation-val').textContent = value.toFixed(2);
            this.sendOSC('/laser/rotation/speed', [value]);
        });
        
        // Position X
        document.getElementById('pos-x').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('pos-x-val').textContent = value.toFixed(2);
            this.sendOSC('/laser/position/x', [value]);
        });
        
        // Position Y
        document.getElementById('pos-y').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('pos-y-val').textContent = value.toFixed(2);
            this.sendOSC('/laser/position/y', [value]);
        });
        
        // Blackout button
        document.getElementById('blackout-btn').addEventListener('click', (e) => {
            const isActive = e.target.classList.toggle('active');
            this.sendOSC('/blackout', [isActive ? 1 : 0]);
        });
    }
    
    setActiveButton(selector, activeBtn) {
        document.querySelectorAll(selector).forEach(btn => {
            btn.classList.remove('active');
        });
        activeBtn.classList.add('active');
    }
    
    async sendOSC(address, args) {
        console.log('OSC:', address, args);
        
        try {
            const response = await fetch(`${this.apiBase}/api/osc`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    address: address,
                    args: args
                })
            });
            
            if (!response.ok) {
                console.error('Failed to send OSC:', response.statusText);
            }
        } catch (error) {
            console.error('Error sending OSC:', error);
        }
    }
    
    async fetchShapes() {
        try {
            const response = await fetch(`${this.apiBase}/api/shapes`);
            const data = await response.json();
            return data;
        } catch (error) {
            console.error('Error fetching shapes:', error);
            return { points: [], blackout: false };
        }
    }
    
    async fetchState() {
        try {
            const response = await fetch(`${this.apiBase}/api/state`);
            const data = await response.json();
            return data;
        } catch (error) {
            console.error('Error fetching state:', error);
            return null;
        }
    }
    
    drawShapes(data) {
        // Clear canvas with dark background
        this.ctx.fillStyle = '#000000';
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
        
        if (data.blackout || !data.points || data.points.length === 0) {
            return;
        }
        
        // Realistic 3D perspective - single beam coming at you
        // No multiple rings - just one shape with depth perception through blur/glow
        
        const centerX = this.canvas.width / 2;
        const centerY = this.canvas.height / 2;
        
        this.ctx.lineCap = 'round';
        this.ctx.lineJoin = 'round';
        
        // Create perspective effect with motion blur trail
        // Draw fading trail from center (far) to current position (near)
        const trailSteps = 8; // Smooth motion blur steps
        
        for (let i = trailSteps; i >= 0; i--) {
            const trailFactor = i / trailSteps; // 0 = current position, 1 = far back
            
            // Scale toward center for depth (further away = smaller, toward center)
            const scale = 1.0 - (trailFactor * 0.4); // 0.6 to 1.0
            
            // Fade trail exponentially - more recent positions are brighter
            const fadeAlpha = Math.pow(1 - trailFactor, 2); // Exponential fade
            
            // Transform points toward center for perspective
            const transformed = data.points.map(([x, y, r, g, b]) => {
                const dx = x - centerX;
                const dy = y - centerY;
                
                // Apply perspective scaling
                const newX = centerX + dx * scale;
                const newY = centerY + dy * scale;
                
                return [newX, newY, r, g, b];
            });
            
            // Draw this trail step with appropriate fade
            // Outer glow for depth
            if (i <= 2) {
                // Most recent positions get intense glow
                this.drawPass(transformed, 15, 0.08 * fadeAlpha);
                this.drawPass(transformed, 10, 0.15 * fadeAlpha);
            }
            this.drawPass(transformed, 6, 0.3 * fadeAlpha);
            this.drawPass(transformed, 3, 0.6 * fadeAlpha);
            this.drawPass(transformed, 1.5, 1.0 * fadeAlpha);
        }
        
        // Draw the main shape at current position (brightest)
        // Intense multi-pass glow for laser beam intensity
        this.drawPass(data.points, 20, 0.05); // Huge outer glow
        this.drawPass(data.points, 12, 0.12); // Outer glow
        this.drawPass(data.points, 8, 0.25);  // Mid glow
        this.drawPass(data.points, 5, 0.5);   // Inner glow
        this.drawPass(data.points, 3, 0.8);   // Bright core
        this.drawPass(data.points, 1.5, 1.0); // Pure beam
        
        // Add vertex sparkle on main shape
        data.points.forEach(([x, y, r, g, b]) => {
            const gradient = this.ctx.createRadialGradient(x, y, 0, x, y, 6);
            gradient.addColorStop(0, `rgba(${r}, ${g}, ${b}, 1.0)`);
            gradient.addColorStop(0.4, `rgba(${r}, ${g}, ${b}, 0.6)`);
            gradient.addColorStop(1, `rgba(${r}, ${g}, ${b}, 0.0)`);
            
            this.ctx.fillStyle = gradient;
            this.ctx.beginPath();
            this.ctx.arc(x, y, 6, 0, Math.PI * 2);
            this.ctx.fill();
        });
        
        // Subtle center bloom for depth atmosphere (much more subtle than before)
        const centerGlow = this.ctx.createRadialGradient(
            centerX, centerY, 0,
            centerX, centerY, Math.min(this.canvas.width, this.canvas.height) * 0.25
        );
        
        if (data.points.length > 0) {
            const [_, __, r, g, b] = data.points[0];
            centerGlow.addColorStop(0, `rgba(${r}, ${g}, ${b}, 0.08)`);
            centerGlow.addColorStop(0.6, `rgba(${r}, ${g}, ${b}, 0.02)`);
            centerGlow.addColorStop(1, `rgba(${r}, ${g}, ${b}, 0.0)`);
            
            this.ctx.fillStyle = centerGlow;
            this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
        }
    }
    
    drawPass(points, lineWidth, alpha) {
        // Draw lines with specified width and alpha
        for (let i = 0; i < points.length - 1; i++) {
            const [x1, y1, r1, g1, b1] = points[i];
            const [x2, y2, r2, g2, b2] = points[i + 1];
            
            // Create gradient for smooth color transitions
            const gradient = this.ctx.createLinearGradient(x1, y1, x2, y2);
            gradient.addColorStop(0, `rgba(${r1}, ${g1}, ${b1}, ${alpha})`);
            gradient.addColorStop(1, `rgba(${r2}, ${g2}, ${b2}, ${alpha})`);
            
            this.ctx.strokeStyle = gradient;
            this.ctx.lineWidth = lineWidth;
            this.ctx.beginPath();
            this.ctx.moveTo(x1, y1);
            this.ctx.lineTo(x2, y2);
            this.ctx.stroke();
        }
    }
    
    async updateCanvas() {
        if (!this.running) return;
        
        const data = await this.fetchShapes();
        this.drawShapes(data);
        
        setTimeout(() => this.updateCanvas(), this.updateInterval);
    }
    
    async updateStatus() {
        try {
            const response = await fetch(`${this.apiBase}/api/status`);
            const data = await response.json();
            
            document.getElementById('osc-port').textContent = data.osc_port;
            
            // Update uptime
            const uptime = Math.floor(data.uptime);
            const hours = Math.floor(uptime / 3600);
            const minutes = Math.floor((uptime % 3600) / 60);
            const seconds = uptime % 60;
            document.getElementById('uptime').textContent = 
                `${hours}:${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
        } catch (error) {
            console.error('Error updating status:', error);
        }
        
        // Update every second
        setTimeout(() => this.updateStatus(), 1000);
    }
    
    async updateStateDisplay() {
        const state = await this.fetchState();
        if (state) {
            document.getElementById('current-shape').textContent = state.current_shape;
            document.getElementById('current-color').textContent = state.current_color;
        }
        
        setTimeout(() => this.updateStateDisplay(), 1000);
    }
    
    startAnimation() {
        this.updateCanvas();
        this.updateStateDisplay();
    }
}

// Initialize UI when page loads
document.addEventListener('DOMContentLoaded', () => {
    new BeamCommanderUI();
});
