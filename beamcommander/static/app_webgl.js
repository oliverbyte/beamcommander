// BeamCommander WebGL 3D Interface with Three.js
// Realistic 3D laser beam visualization with lighting, fog, and reflections

class BeamCommander3D {
    constructor() {
        this.container = document.getElementById('laser-canvas');
        this.apiBase = '';
        this.updateInterval = 50; // 20 FPS
        this.running = true;
        
        this.initThreeJS();
        this.setupEventListeners();
        this.startAnimation();
        this.updateStatus();
    }
    
    initThreeJS() {
        // Create scene
        this.scene = new THREE.Scene();
        this.scene.fog = new THREE.FogExp2(0x000000, 0.002); // Atmospheric fog
        
        // Setup camera (perspective for 3D depth)
        const aspect = this.container.clientWidth / this.container.clientHeight;
        this.camera = new THREE.PerspectiveCamera(75, aspect, 0.1, 1000);
        this.camera.position.set(0, 0, 15); // Looking into the scene
        this.camera.lookAt(0, 0, 0);
        
        // Create renderer with antialiasing
        this.renderer = new THREE.WebGLRenderer({ 
            antialias: true,
            alpha: true
        });
        this.renderer.setSize(this.container.clientWidth, this.container.clientHeight);
        this.renderer.setPixelRatio(window.devicePixelRatio);
        this.container.appendChild(this.renderer.domElement);
        
        // Setup lighting
        this.setupLighting();
        
        // Initialize laser beam geometry
        this.laserGroup = new THREE.Group();
        this.scene.add(this.laserGroup);
        
        // Handle window resize
        window.addEventListener('resize', () => this.onWindowResize());
    }
    
    setupLighting() {
        // Ambient light for base visibility
        const ambient = new THREE.AmbientLight(0x222222);
        this.scene.add(ambient);
        
        // Point light at camera position (represents viewer's perspective)
        this.viewLight = new THREE.PointLight(0xffffff, 0.3, 100);
        this.viewLight.position.copy(this.camera.position);
        this.scene.add(this.viewLight);
        
        // Dynamic lights that will follow laser beams
        this.beamLights = [];
        for (let i = 0; i < 3; i++) {
            const light = new THREE.PointLight(0x0000ff, 2, 20);
            light.position.set(0, 0, 0);
            this.scene.add(light);
            this.beamLights.push(light);
        }
    }
    
    createLaserBeam(points, color) {
        // Clear previous geometry
        while (this.laserGroup.children.length > 0) {
            this.laserGroup.remove(this.laserGroup.children[0]);
        }
        
        if (!points || points.length === 0) return;
        
        // Convert 2D points to 3D with depth
        const vertices = [];
        const colors = [];
        
        for (let i = 0; i < points.length; i++) {
            const [x, y, r, g, b] = points[i];
            
            // Convert from canvas coordinates to 3D space
            const x3d = (x - this.container.clientWidth / 2) / 50;
            const y3d = -(y - this.container.clientHeight / 2) / 50;
            const z3d = 0; // Main beam at z=0
            
            vertices.push(x3d, y3d, z3d);
            colors.push(r / 255, g / 255, b / 255);
        }
        
        // Create core beam (thin, bright)
        this.createBeamPass(vertices, colors, 0.15, 1.0, true);
        
        // Create glow layers (wider, transparent)
        this.createBeamPass(vertices, colors, 0.4, 0.4, false);
        this.createBeamPass(vertices, colors, 0.8, 0.2, false);
        this.createBeamPass(vertices, colors, 1.5, 0.1, false);
        
        // Add volumetric light effect
        this.addVolumetricEffect(vertices, colors);
        
        // Update dynamic lights to match beam
        this.updateBeamLights(vertices, colors);
    }
    
    createBeamPass(vertices, colors, lineWidth, opacity, additive) {
        const geometry = new THREE.BufferGeometry();
        geometry.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geometry.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
        
        const material = new THREE.LineBasicMaterial({
            vertexColors: true,
            linewidth: lineWidth,
            opacity: opacity,
            transparent: true,
            blending: additive ? THREE.AdditiveBlending : THREE.NormalBlending
        });
        
        const line = new THREE.Line(geometry, material);
        this.laserGroup.add(line);
    }
    
    addVolumetricEffect(vertices, colors) {
        // Create glowing spheres at each vertex for volumetric light effect
        for (let i = 0; i < vertices.length; i += 3) {
            const x = vertices[i];
            const y = vertices[i + 1];
            const z = vertices[i + 2];
            const r = colors[i];
            const g = colors[i + 1];
            const b = colors[i + 2];
            
            // Create small sphere with emissive material
            const geometry = new THREE.SphereGeometry(0.2, 8, 8);
            const material = new THREE.MeshBasicMaterial({
                color: new THREE.Color(r, g, b),
                transparent: true,
                opacity: 0.6,
                blending: THREE.AdditiveBlending
            });
            
            const sphere = new THREE.Mesh(geometry, material);
            sphere.position.set(x, y, z);
            this.laserGroup.add(sphere);
        }
    }
    
    updateBeamLights(vertices, colors) {
        // Position lights at key points along the beam
        const positions = [0, Math.floor(vertices.length / 6), Math.floor(vertices.length / 3)];
        
        positions.forEach((idx, i) => {
            if (idx < vertices.length && this.beamLights[i]) {
                const realIdx = idx * 3;
                this.beamLights[i].position.set(
                    vertices[realIdx],
                    vertices[realIdx + 1],
                    vertices[realIdx + 2]
                );
                
                // Set light color to match beam
                if (colors[realIdx] !== undefined) {
                    this.beamLights[i].color.setRGB(
                        colors[realIdx],
                        colors[realIdx + 1],
                        colors[realIdx + 2]
                    );
                }
            }
        });
    }
    
    onWindowResize() {
        const width = this.container.clientWidth;
        const height = this.container.clientHeight;
        
        this.camera.aspect = width / height;
        this.camera.updateProjectionMatrix();
        this.renderer.setSize(width, height);
    }
    
    render() {
        // Subtle camera animation for depth perception
        const time = Date.now() * 0.0001;
        this.camera.position.x = Math.sin(time) * 0.5;
        this.camera.position.y = Math.cos(time * 0.7) * 0.3;
        this.camera.lookAt(0, 0, 0);
        
        // Rotate laser group slightly for 3D effect
        this.laserGroup.rotation.z = Math.sin(time * 0.5) * 0.02;
        
        this.renderer.render(this.scene, this.camera);
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
            document.getElementById('dot-amount-val').textContent = e.target.value + '%';
            this.sendOSC('/laser/points', [value]);
        });
        
        // Rainbow sliders
        document.getElementById('rainbow-amount').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('rainbow-amount-val').textContent = e.target.value + '%';
            this.sendOSC('/rainbow/amount', [value]);
        });
        
        document.getElementById('rainbow-speed').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('rainbow-speed-val').textContent = e.target.value;
            this.sendOSC('/rainbow/speed', [value]);
        });
        
        // Movement sliders
        document.getElementById('move-size').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('move-size-val').textContent = e.target.value + '%';
            this.sendOSC('/move/size', [value]);
        });
        
        document.getElementById('move-speed').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('move-speed-val').textContent = value.toFixed(1);
            this.sendOSC('/move/speed', [value]);
        });
        
        // Transform sliders
        document.getElementById('scale').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('scale-val').textContent = e.target.value;
            this.sendOSC('/transform/scale', [value]);
        });
        
        document.getElementById('rotation').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('rotation-val').textContent = e.target.value;
            this.sendOSC('/transform/rotation', [value]);
        });
        
        document.getElementById('pos-x').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('pos-x-val').textContent = e.target.value;
            this.sendOSC('/transform/x', [value]);
        });
        
        document.getElementById('pos-y').addEventListener('input', (e) => {
            const value = e.target.value / 100;
            document.getElementById('pos-y-val').textContent = e.target.value;
            this.sendOSC('/transform/y', [value]);
        });
        
        // Blackout button
        document.getElementById('blackout').addEventListener('click', (e) => {
            e.target.classList.toggle('active');
            const isActive = e.target.classList.contains('active');
            this.sendOSC('/laser/blackout', [isActive ? 1 : 0]);
        });
    }
    
    setActiveButton(selector, activeBtn) {
        document.querySelectorAll(selector).forEach(btn => btn.classList.remove('active'));
        activeBtn.classList.add('active');
    }
    
    async sendOSC(address, args) {
        try {
            console.log(`OSC: ${address} [${args.join(', ')}]`);
            const response = await fetch(`${this.apiBase}/api/osc`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ address, args })
            });
            
            if (!response.ok) {
                console.error('OSC send failed:', response.statusText);
            }
        } catch (error) {
            console.error('Error sending OSC:', error);
        }
    }
    
    async updateShapes() {
        try {
            const response = await fetch(`${this.apiBase}/api/shapes`);
            const data = await response.json();
            this.createLaserBeam(data.points, data.color);
        } catch (error) {
            console.error('Error fetching shapes:', error);
        }
    }
    
    async updateStatus() {
        try {
            const response = await fetch(`${this.apiBase}/api/status`);
            const data = await response.json();
            
            document.querySelector('.status strong:nth-child(2)').textContent = data.osc_port;
            
            const uptime = Math.floor(data.uptime);
            const hours = Math.floor(uptime / 3600);
            const minutes = Math.floor((uptime % 3600) / 60);
            const seconds = uptime % 60;
            document.querySelectorAll('.status strong')[1].textContent = 
                `${hours}:${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
                
        } catch (error) {
            console.error('Error updating status:', error);
        }
        
        if (this.running) {
            setTimeout(() => this.updateStatus(), 1000);
        }
    }
    
    startAnimation() {
        const animate = () => {
            if (!this.running) return;
            
            requestAnimationFrame(animate);
            this.render();
        };
        
        animate();
        
        // Update shapes from server
        setInterval(() => {
            if (this.running) {
                this.updateShapes();
            }
        }, this.updateInterval);
    }
    
    destroy() {
        this.running = false;
        this.renderer.dispose();
    }
}

// Initialize when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
    window.beamCommander = new BeamCommander3D();
});
