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
        // In a real implementation, this would send OSC via WebSocket or HTTP proxy
        // For now, we'll just log it
        console.log('OSC:', address, args);
        
        // Note: Since we can't send OSC directly from browser,
        // this would need a WebSocket bridge or HTTP-to-OSC proxy
        // For demonstration, we're showing the UI only
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
        // Clear canvas
        this.ctx.fillStyle = '#000000';
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
        
        if (data.blackout || !data.points || data.points.length === 0) {
            return;
        }
        
        // Draw points
        this.ctx.lineCap = 'round';
        this.ctx.lineJoin = 'round';
        
        for (let i = 0; i < data.points.length - 1; i++) {
            const [x1, y1, r1, g1, b1] = data.points[i];
            const [x2, y2, r2, g2, b2] = data.points[i + 1];
            
            // Create gradient for smooth color transitions
            const gradient = this.ctx.createLinearGradient(x1, y1, x2, y2);
            gradient.addColorStop(0, `rgb(${r1}, ${g1}, ${b1})`);
            gradient.addColorStop(1, `rgb(${r2}, ${g2}, ${b2})`);
            
            this.ctx.strokeStyle = gradient;
            this.ctx.lineWidth = 2;
            this.ctx.beginPath();
            this.ctx.moveTo(x1, y1);
            this.ctx.lineTo(x2, y2);
            this.ctx.stroke();
        }
        
        // Draw points for dotted effect
        data.points.forEach(([x, y, r, g, b]) => {
            this.ctx.fillStyle = `rgb(${r}, ${g}, ${b})`;
            this.ctx.beginPath();
            this.ctx.arc(x, y, 1, 0, Math.PI * 2);
            this.ctx.fill();
        });
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
