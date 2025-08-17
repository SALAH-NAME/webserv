document.head.insertAdjacentHTML('beforeend', `
    <style id="preload-header-style">
        .header {
            opacity: 0 !important;
            transform: translateY(-30px) scale(0.9) !important;
            filter: blur(5px) !important;
        }
    </style>
`);

if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initializeApp);
} else {
    
    initializeApp();
}

function initializeApp() {
    const LOAD_DELAY_BASE = 5; 
    const LOAD_DELAY_VARIANCE = 15; 
    const GLITCH_LOADING_DURATION = 50; 
    const INTERSECTION_THRESHOLD = 0.1;
    const ROOT_MARGIN = '0px 0px -50px 0px';
    
    
    let loadedSections = new Set();
    let loadingQueue = [];
    let isProcessingQueue = false;
    
    const cardButtons = document.querySelectorAll('.card-button');
    cardButtons.forEach(button => {
        const buttonText = button.textContent.trim().toUpperCase();
        button.setAttribute('data-text', buttonText);
    });
    
    const sectionObserver = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                const section = entry.target;
                const sectionTitle = section.querySelector('.section-title');
                
                if (!loadedSections.has(section)) {
                    loadedSections.add(section);
                    
                    if (sectionTitle) {
                        glitchElement(sectionTitle, 400);
                    }
                    
                    loadCardsInSection(section);
                }
            }
        });
    }, {
        threshold: INTERSECTION_THRESHOLD,
        rootMargin: ROOT_MARGIN
    });
    
    const cardObserver = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting && entry.target.classList.contains('hidden-card')) {
                queueCardLoad(entry.target);
            }
        });
    }, {
        threshold: 0.05,
        rootMargin: '0px 0px -50px 0px'
    });
    
    function queueCardLoad(card) {
        if (!loadingQueue.includes(card)) {
            loadingQueue.push(card);
            processQueue();
        }
    }
    
    async function processQueue() {
        if (isProcessingQueue || loadingQueue.length === 0) return;
        
        isProcessingQueue = true;
        
        while (loadingQueue.length > 0) {
            const card = loadingQueue.shift();
            if (card && card.classList.contains('hidden-card')) {
                await loadCardWithGlitch(card);
                await new Promise(resolve => 
                    setTimeout(resolve, LOAD_DELAY_BASE + Math.random() * LOAD_DELAY_VARIANCE)
                );
            }
        }
        
        isProcessingQueue = false;
    }
    
    function loadCardWithGlitch(card) {
        return new Promise((resolve) => {
            card.classList.add('loading');
            card.classList.remove('hidden-card');
            
            const glitchInterval = setInterval(() => {
                if (Math.random() < 0.7) {
                    const randomTransform = `
                        translateY(${Math.random() * 40 - 20}px) 
                        scale(${0.85 + Math.random() * 0.3}) 
                        skew(${Math.random() * 6 - 3}deg)
                    `;
                    const randomFilter = `
                        blur(${Math.random() * 4}px) 
                        hue-rotate(${Math.random() * 360}deg) 
                        saturate(${100 + Math.random() * 200}%) 
                        contrast(${80 + Math.random() * 140}%)
                    `;
                    
                    card.style.transform = randomTransform;
                    card.style.filter = randomFilter;
                    
                    setTimeout(() => {
                        card.style.transform = '';
                        card.style.filter = '';
                    }, 30 + Math.random() * 50);
                }
            }, 150);
            
            setTimeout(() => {
                clearInterval(glitchInterval);
                card.classList.remove('loading');
                card.classList.add('loaded');
                
                setupCardInteractions(card);
                
                const teamImage = card.querySelector('.team-member-image');
                if (teamImage) {
                    setupImageGlitch(teamImage);
                }
                
                resolve();
            }, GLITCH_LOADING_DURATION);
        });
    }
    
    function loadCardsInSection(section) {
        const cards = section.querySelectorAll('.hidden-card');
        cards.forEach((card, index) => {
            if (card.classList.contains('section-title')) {
                card.classList.remove('hidden-card');
                card.classList.add('loaded');
                glitchElement(card, 50);
            } else {
                cardObserver.observe(card);
            }
        });
    }
    
    function setupCardInteractions(card) {
        card.addEventListener('mouseenter', function() {
            if (Math.random() < 0.6) {
                glitchElement(this, 200);
            }
        });
        
        function microGlitch() {
            if (Math.random() < 0.12 && !card.matches(':hover')) {
                const glitchType = Math.random();
                
                if (glitchType < 0.3) {
                    card.classList.add('glitch-active');
                    setTimeout(() => card.classList.remove('glitch-active'), 400);
                } else if (glitchType < 0.6) {
                    card.classList.add('scanlines');
                    setTimeout(() => card.classList.remove('scanlines'), 1000);
                } else {
                    card.style.transform = `translate(${Math.random() * 4 - 2}px, ${Math.random() * 4 - 2}px)`;
                    card.style.filter = `hue-rotate(${Math.random() * 60 - 30}deg) saturate(${100 + Math.random() * 100}%)`;
                    
                    setTimeout(() => {
                        card.style.transform = '';
                        card.style.filter = '';
                    }, 100);
                }
            }
        }
        
        setInterval(microGlitch, Math.random() * 1500 + 1000);
        
        const button = card.querySelector('.card-button');
        if (button) {
            setupButtonInteractions(button);
        }
    }
    
    function setupButtonInteractions(button) {
        button.addEventListener('focus', function() {
            this.style.outline = '3px solid var(--color-primary-pink)';
            this.style.outlineOffset = '3px';
            this.style.boxShadow = 'var(--shadow-button)';
            this.style.transform = 'scale(1.02)';
            this.style.textShadow = '1px 0 var(--color-primary-cyan), -1px 0 var(--color-primary-pink)';
        });
        
        button.addEventListener('blur', function() {
            this.style.outline = 'none';
            this.style.boxShadow = 'none';
            this.style.transform = '';
            this.style.textShadow = '';
        });
        
        button.addEventListener('click', function(e) {
            e.stopPropagation();
            
            this.style.transform = 'scale(0.95) translate(3px, -3px)';
            this.style.filter = 'hue-rotate(180deg) saturate(150%)';
            this.style.textShadow = '2px 0 var(--color-accent-yellow), -2px 0 var(--color-primary-cyan)';
            
            setTimeout(() => {
                this.style.transform = '';
                this.style.filter = '';
                this.style.textShadow = '';
            }, 150);
        });
        
        button.addEventListener('keydown', function(e) {
            if (e.key === 'Enter' || e.key === ' ') {
                glitchElement(this, 300);
            }
        });
    }
    
    function setupImageGlitch(img) {
        img.addEventListener('load', function() {
            this.classList.add('loaded');
            glitchElement(this, 300);
        });
        
        img.addEventListener('error', function() {
            this.src = generateGlitchPlaceholder();
            this.classList.add('loaded');
        });
        
        if (img.complete) {
            img.classList.add('loaded');
        }
    }
    
    function generateGlitchPlaceholder() {
        return 'data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTUwIiBoZWlnaHQ9IjE1MCIgdmlld0JveD0iMCAwIDE1MCAxNTAiIGZpbGw9Im5vbmUiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyI+CjxyZWN0IHdpZHRoPSIxNTAiIGhlaWdodD0iMTUwIiBmaWxsPSIjMzMzIi8+Cjx0ZXh0IHg9Ijc1IiB5PSI3NSIgdGV4dC1hbmNob3I9Im1pZGRsZSIgZG9taW5hbnQtYmFzZWxpbmU9ImNlbnRyYWwiIGZpbGw9IiNmZjAwODAiIGZvbnQtZmFtaWx5PSJKZXRCcmFpbnMgTW9ubyIgZm9udC1zaXplPSIxNCIgZm9udC13ZWlnaHQ9IjcwMCI+VGVhbTwvdGV4dD4KPHRleHQgeD0iNzUiIHk9Ijk1IiB0ZXh0LWFuY2hvcj0ibWlkZGxlIiBkb21pbmFudC1iYXNlbGluZT0iY2VudHJhbCIgZmlsbD0iIzAwZmZmZiIgZm9udC1mYW1pbHk9IkpldEJyYWlucyBNb25vIiBmb250LXNpemU9IjE0IiBmb250LXdlaWdodD0iNzAwIj5NZW1iZXI8L3RleHQ+Cjwvc3ZnPg==';
    }
    
    function applyRandomGlitchVariation(element, variation, duration = 100) {
        if (!element || !element.style) return;
        
        const originalTransform = element.style.transform;
        const originalFilter = element.style.filter;
        const originalTextShadow = element.style.textShadow;
        const originalClipPath = element.style.clipPath;
        const originalBackground = element.style.background;
        
        try {
            switch (variation) {
                case 'dataCorruption':
                    applyDataCorruptionGlitch(element, duration);
                    break;
                case 'chromatic':
                    applyChromaticGlitch(element, duration);
                    break;
                case 'digitalNoise':
                    applyDigitalNoiseGlitch(element, duration);
                    break;
                case 'scanlines':
                    applyScanlineGlitch(element, duration);
                    break;
                case 'fragmentShift':
                    applyFragmentShiftGlitch(element, duration);
                    break;
                case 'colorSplit':
                    applyColorSplitGlitch(element, duration);
                    break;
                case 'staticFlicker':
                    applyStaticFlickerGlitch(element, duration);
                    break;
                case 'matrixGlitch':
                    applyMatrixGlitch(element, duration);
                    break;
                default:
                    glitchElement(element, duration);
            }
        } catch (error) {
            console.warn('Glitch effect error:', error);
            glitchElement(element, duration);
        }
        
        setTimeout(() => {
            try {
                if (element && element.style) {
                    element.style.transform = originalTransform;
                    element.style.filter = originalFilter;
                    element.style.textShadow = originalTextShadow;
                    element.style.clipPath = originalClipPath;
                    element.style.background = originalBackground;
                }
            } catch (error) {
                console.warn('Glitch cleanup error:', error);
            }
        }, duration + 100);
    }
    
    function applyDataCorruptionGlitch(element, duration) {
        if (!element) return;
        
        const intervals = [];
        
        intervals.push(setInterval(() => {
            if (!element.style) return;
            element.style.transform = `
                translate(${Math.random() * 20 - 10}px, ${Math.random() * 8 - 4}px) 
                scale(${0.95 + Math.random() * 0.1})
                skew(${Math.random() * 10 - 5}deg)
            `;
        }, 80));
        
        intervals.push(setInterval(() => {
            if (!element.style) return;
            element.style.filter = `
                hue-rotate(${Math.random() * 360}deg) 
                saturate(${300 + Math.random() * 200}%) 
                contrast(${200 + Math.random() * 100}%)
                ${Math.random() < 0.3 ? 'invert(1)' : ''}
            `;
            element.style.willChange = 'filter';
        }, 120));
        
        intervals.push(setInterval(() => {
            if (!element.style) return;
            const colors = ['#ff0080', '#00ffff', '#ff0040', '#40ff00', '#ff4000'];
            const color1 = colors[Math.floor(Math.random() * colors.length)];
            const color2 = colors[Math.floor(Math.random() * colors.length)];
            
            element.style.textShadow = `
                ${Math.random() * 15}px ${Math.random() * 10 - 5}px ${color1},
                ${Math.random() * -15}px ${Math.random() * 10 - 5}px ${color2},
                ${Math.random() * 10 - 5}px ${Math.random() * 15}px ${color1},
                ${Math.random() * 10 - 5}px ${Math.random() * -15}px ${color2}
            `;
        }, 60));
        
        setTimeout(() => {
            intervals.forEach(clearInterval);
            if (element && element.style) {
                element.style.willChange = 'auto'; 
            }
        }, duration);
    }
    
    function applyChromaticGlitch(element, duration) {
        const intervals = [];
        
        intervals.push(setInterval(() => {
            const offset = Math.random() * 8 + 2;
            element.style.textShadow = `
                ${offset}px 0 #ff0080,
                ${-offset}px 0 #00ffff,
                0 ${offset}px #ff4000,
                0 ${-offset}px #40ff00
            `;
            element.style.filter = `contrast(150%) saturate(200%)`;
        }, 100));
        
        setTimeout(() => intervals.forEach(clearInterval), duration);
    }
    
    function applyDigitalNoiseGlitch(element, duration) {
        const intervals = [];
        
        intervals.push(setInterval(() => {
            element.style.filter = `
                contrast(${100 + Math.random() * 200}%)
                brightness(${80 + Math.random() * 140}%)
                saturate(${150 + Math.random() * 150}%)
            `;
            element.style.transform = `
                translate(${Math.random() * 6 - 3}px, ${Math.random() * 6 - 3}px)
                scale(${0.98 + Math.random() * 0.04})
            `;
        }, 40));
        
        setTimeout(() => intervals.forEach(clearInterval), duration);
    }
    
    function applyScanlineGlitch(element, duration) {
        const overlay = document.createElement('div');
        overlay.style.cssText = `
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: repeating-linear-gradient(
                0deg,
                transparent,
                transparent 2px,
                rgba(255, 0, 128, 0.1) 2px,
                rgba(255, 0, 128, 0.1) 4px
            );
            pointer-events: none;
            z-index: 10;
            animation: scanlineMove 0.1s linear infinite;
        `;
        
        element.style.position = 'relative';
        element.appendChild(overlay);
        
        setTimeout(() => {
            if (overlay.parentNode) overlay.remove();
        }, duration);
    }
    
    function applyFragmentShiftGlitch(element, duration) {
        const intervals = [];
        
        intervals.push(setInterval(() => {
            const clips = [
                `inset(${Math.random() * 50}% 0 ${Math.random() * 50}% 0)`,
                `inset(0 ${Math.random() * 30}% 0 ${Math.random() * 30}%)`,
                `polygon(0 0, ${50 + Math.random() * 50}% 0, 100% ${Math.random() * 100}%, 0 100%)`
            ];
            element.style.clipPath = clips[Math.floor(Math.random() * clips.length)];
        }, 150));
        
        setTimeout(() => {
            intervals.forEach(clearInterval);
            element.style.clipPath = '';
        }, duration);
    }
    
    function applyColorSplitGlitch(element, duration) {
        const intervals = [];
        
        intervals.push(setInterval(() => {
            element.style.background = `
                linear-gradient(
                    ${Math.random() * 360}deg,
                    #ff0080 ${Math.random() * 30}%,
                    transparent ${30 + Math.random() * 20}%,
                    #00ffff ${50 + Math.random() * 30}%,
                    transparent ${80 + Math.random() * 20}%
                )
            `;
            element.style.backgroundClip = 'text';
            element.style.webkitBackgroundClip = 'text';
            element.style.color = 'transparent';
        }, 120));
        
        setTimeout(() => {
            intervals.forEach(clearInterval);
            element.style.background = '';
            element.style.backgroundClip = '';
            element.style.webkitBackgroundClip = '';
            element.style.color = '';
        }, duration);
    }
    
    function applyStaticFlickerGlitch(element, duration) {
        const intervals = [];
        
        intervals.push(setInterval(() => {
            element.style.opacity = Math.random() < 0.1 ? '0.1' : '1';
            element.style.filter = `
                brightness(${Math.random() < 0.2 ? 300 : 100}%)
                contrast(${Math.random() < 0.2 ? 300 : 100}%)
            `;
        }, 50));
        
        setTimeout(() => {
            intervals.forEach(clearInterval);
            element.style.opacity = '';
        }, duration);
    }
    
    function applyMatrixGlitch(element, duration) {
        const originalText = element.textContent;
        const chars = '01アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヲン';
        const intervals = [];
        
        intervals.push(setInterval(() => {
            let glitchedText = '';
            for (let i = 0; i < originalText.length; i++) {
                if (Math.random() < 0.1) {
                    glitchedText += chars[Math.floor(Math.random() * chars.length)];
                } else {
                    glitchedText += originalText[i];
                }
            }
            element.textContent = glitchedText;
        }, 100));
        
        setTimeout(() => {
            intervals.forEach(clearInterval);
            element.textContent = originalText;
        }, duration);
    }
    
    function applyAmbientGlitch(element, variation) {
        if (!element || !element.style) return;
        
        const duration = 100 + Math.random() * 150;
        
        try {
            switch (variation) {
                case 'subtle':
                    element.style.textShadow = `1px 0 rgba(255, 0, 128, 0.3), -1px 0 rgba(0, 255, 255, 0.3)`;
                    setTimeout(() => {
                        if (element && element.style) element.style.textShadow = '';
                    }, duration);
                    break;
                case 'whisper':
                    element.style.transform = `translate(${Math.random() * 2 - 1}px, ${Math.random() * 2 - 1}px)`;
                    setTimeout(() => {
                        if (element && element.style) element.style.transform = '';
                    }, duration);
                    break;
                case 'breath':
                    element.style.filter = `saturate(${120 + Math.random() * 30}%)`;
                    setTimeout(() => {
                        if (element && element.style) element.style.filter = '';
                    }, duration);
                    break;
            }
        } catch (error) {
            console.warn('Ambient glitch error:', error);
        }
    }

    let activeGlitchCount = 0;
    const MAX_CONCURRENT_GLITCHES = 5;
    
    function canRunGlitch() {
        return activeGlitchCount < MAX_CONCURRENT_GLITCHES;
    }
    
    function incrementGlitchCount() {
        activeGlitchCount++;
    }
    
    function decrementGlitchCount() {
        activeGlitchCount = Math.max(0, activeGlitchCount - 1);
    }
    
    function performanceAwareGlitch(element, variation, duration) {
        if (!canRunGlitch()) {
            return; 
        }
        
        incrementGlitchCount();
        applyRandomGlitchVariation(element, variation, duration);
        
        setTimeout(() => {
            decrementGlitchCount();
        }, duration + 100);
    }

    function glitchElement(element, duration = 400) {
        const glitchColors = ['var(--color-primary-pink)', 'var(--color-primary-cyan)', 'var(--color-accent-yellow)'];
        
        const originalTransform = element.style.transform;
        const originalFilter = element.style.filter;
        const originalTextShadow = element.style.textShadow;
        
        const glitchInterval = setInterval(() => {
            const color1 = glitchColors[Math.floor(Math.random() * glitchColors.length)];
            const color2 = glitchColors[Math.floor(Math.random() * glitchColors.length)];
            const intensity = Math.random() * 4 + 1;
            
            element.style.textShadow = `
                ${intensity}px 0 ${color1},
                -${intensity}px 0 ${color2},
                0 ${intensity}px ${color1},
                0 -${intensity}px ${color2}
            `;
            element.style.filter = `hue-rotate(${Math.random() * 360}deg) saturate(${100 + Math.random() * 100}%)`;
            element.style.transform = `translate(${Math.random() * 4 - 2}px, ${Math.random() * 4 - 2}px) scale(${0.98 + Math.random() * 0.04})`;
            
            setTimeout(() => {
                element.style.textShadow = originalTextShadow;
                element.style.filter = originalFilter;
                element.style.transform = originalTransform;
            }, 50 + Math.random() * 100);
        }, 80);
        
        setTimeout(() => {
            clearInterval(glitchInterval);
            element.style.transform = originalTransform;
            element.style.filter = originalFilter;
            element.style.textShadow = originalTextShadow;
        }, duration);
    }
    
    function initializeTitleEffects() {
        const titles = document.querySelectorAll('.glitch-text');
        const sectionTitles = document.querySelectorAll('.section-title');
        
        const prefersReducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)').matches;
        
        const glitchVariations = [
            'dataCorruption',
            'chromatic',
            'digitalNoise', 
            'scanlines',
            'fragmentShift',
            'colorSplit',
            'staticFlicker',
            'matrixGlitch'
        ];
        
        const reducedGlitchVariations = ['chromatic', 'subtle', 'whisper'];
        const activeVariations = prefersReducedMotion ? reducedGlitchVariations : glitchVariations;
        
        titles.forEach(title => {
            function randomGlitchTitle() {
                const frequency = prefersReducedMotion ? 0.15 : 0.4;
                if (Math.random() < frequency && canRunGlitch()) {
                    const variation = activeVariations[Math.floor(Math.random() * activeVariations.length)];
                    const duration = prefersReducedMotion ? 150 : (250 + Math.random() * 200);
                    performanceAwareGlitch(title, variation, duration);
                }
            }
            
            const interval = prefersReducedMotion ? 300 : (120 + Math.random() * 180);
            setInterval(randomGlitchTitle, interval);
        });
        
        sectionTitles.forEach(sectionTitle => {
            function randomGlitchSection() {
                const frequency = prefersReducedMotion ? 0.1 : 0.2;
                if (Math.random() < frequency && canRunGlitch()) {
                    const variation = activeVariations[Math.floor(Math.random() * activeVariations.length)];
                    const duration = prefersReducedMotion ? 100 : (180 + Math.random() * 150);
                    performanceAwareGlitch(sectionTitle, variation, duration);
                }
            }
            
            const interval = prefersReducedMotion ? 500 : (200 + Math.random() * 300);
            setInterval(randomGlitchSection, interval);
        });
        
        if (!prefersReducedMotion) {
            function ambientHeaderGlitch() {
                if (Math.random() < 0.08) {
                    const headerTitles = document.querySelectorAll('.header .glitch-text');
                    headerTitles.forEach(title => {
                        const ambientVariations = ['subtle', 'whisper', 'breath'];
                        const variation = ambientVariations[Math.floor(Math.random() * ambientVariations.length)];
                        applyAmbientGlitch(title, variation);
                    });
                }
            }
            
            setInterval(ambientHeaderGlitch, 50 + Math.random() * 100);
        }
    }
    
    function initializeBackgroundEffects() {
        function randomBackgroundGlitch() {
            if (Math.random() < 0.08) { 
                const hueShift = Math.random() * 60 - 30;
                const saturation = Math.random() * 50 + 100;
                
                document.body.style.filter = `hue-rotate(${hueShift}deg) saturate(${saturation}%)`;
                
                setTimeout(() => {
                    document.body.style.filter = 'none';
                }, 100 + Math.random() * 200);
            }
        }
        
        setInterval(randomBackgroundGlitch, 1000 + Math.random() * 2000); 
        
        function pageWideGlitch() {
            if (Math.random() < 0.015) { 
                const container = document.querySelector('.container');
                container.style.transform = `translate(${Math.random() * 4 - 2}px, ${Math.random() * 4 - 2}px)`;
                container.style.filter = `contrast(${Math.random() * 50 + 100}%) saturate(${Math.random() * 100 + 100}%)`;
                
                setTimeout(() => {
                    container.style.transform = '';
                    container.style.filter = '';
                }, 50 + Math.random() * 100);
            }
        }
        
        setInterval(pageWideGlitch, 500); 
    }
    
    function fastGlitchElement(element, duration = 80) {
        const glitchColors = ['var(--color-primary-pink)', 'var(--color-primary-cyan)', 'var(--color-accent-yellow)'];
        
        const originalTransform = element.style.transform;
        const originalFilter = element.style.filter;
        const originalTextShadow = element.style.textShadow;
        
        element.style.textShadow = `
            8px 0 var(--color-primary-pink),
            -8px 0 var(--color-primary-cyan),
            0 8px var(--color-accent-yellow),
            0 -8px var(--color-primary-pink)
        `;
        element.style.filter = `hue-rotate(180deg) saturate(300%) contrast(200%) brightness(150%)`;
        element.style.transform = `translate(6px, -6px) scale(1.05)`;
        
        setTimeout(() => {
            element.style.textShadow = originalTextShadow;
            element.style.filter = originalFilter;
            element.style.transform = originalTransform;
        }, 50);
        
        const glitchInterval = setInterval(() => {
            const color1 = glitchColors[Math.floor(Math.random() * glitchColors.length)];
            const color2 = glitchColors[Math.floor(Math.random() * glitchColors.length)];
            const intensity = Math.random() * 8 + 3;
            
            element.style.textShadow = `
                ${intensity}px 0 ${color1},
                -${intensity}px 0 ${color2},
                0 ${intensity}px ${color1},
                0 -${intensity}px ${color2}
            `;
            element.style.filter = `hue-rotate(${Math.random() * 360}deg) saturate(${200 + Math.random() * 150}%) contrast(${150 + Math.random() * 100}%)`;
            element.style.transform = `translate(${Math.random() * 8 - 4}px, ${Math.random() * 8 - 4}px) scale(${0.95 + Math.random() * 0.1})`;
            
            setTimeout(() => {
                element.style.textShadow = originalTextShadow;
                element.style.filter = originalFilter;
                element.style.transform = originalTransform;
            }, 15 + Math.random() * 25); 
        }, 35); 
        
        setTimeout(() => {
            clearInterval(glitchInterval);
            element.style.transform = originalTransform;
            element.style.filter = originalFilter;
            element.style.textShadow = originalTextShadow;
        }, duration);
    }

    function loadHeaderWithGlitch() {
        const header = document.querySelector('.header');
        const titles = document.querySelectorAll('.glitch-text');
        const subtitle = document.querySelector('.subtitle');
        
        const preloadStyle = document.getElementById('preload-header-style');
        if (preloadStyle) {
            preloadStyle.remove();
        }
        
        if (header) {
            header.style.transition = 'all 0.2s cubic-bezier(0.175, 0.885, 0.32, 1.275)';
            header.style.opacity = '1';
            header.style.transform = 'translateY(0) scale(1)';
            header.style.filter = 'blur(0)';
            
            header.style.background = 'radial-gradient(circle at center, rgba(255, 0, 128, 0.1) 0%, transparent 70%)';
            setTimeout(() => {
                header.style.background = '';
            }, 1000);
        }
        
        const spectacularGlitchVariations = [
            'dataCorruption', 'chromatic', 'colorSplit', 'fragmentShift', 
            'matrixGlitch', 'staticFlicker', 'digitalNoise'
        ];
        
        titles.forEach((title, index) => {
            setTimeout(() => {
                title.style.transform = 'scale(1.2) rotate(2deg)';
                title.style.filter = 'contrast(200%) saturate(300%) brightness(150%)';
                title.style.textShadow = '5px 0 #ff0080, -5px 0 #00ffff, 0 5px #ffff00';
                
                setTimeout(() => {
                    title.style.transform = 'scale(1.05)';
                    title.style.filter = 'contrast(120%) saturate(150%)';
                    title.style.textShadow = '2px 0 #ff0080, -2px 0 #00ffff';
                }, 150);
                
                setTimeout(() => {
                    title.style.transform = '';
                    title.style.filter = '';
                    title.style.textShadow = '';
                }, 300);
                
                const variation1 = spectacularGlitchVariations[Math.floor(Math.random() * spectacularGlitchVariations.length)];
                const variation2 = spectacularGlitchVariations[Math.floor(Math.random() * spectacularGlitchVariations.length)];
                
                setTimeout(() => {
                    applyRandomGlitchVariation(title, variation1, 200);
                }, 100);
                
                setTimeout(() => {
                    applyRandomGlitchVariation(title, variation2, 180);
                }, 350);
                
                setTimeout(() => {
                    fastGlitchElement(title, 150);
                }, 600);
                
            }, index * 200); 
        });
        
        if (subtitle) {
            setTimeout(() => {
                subtitle.style.transform = 'scale(1.1) translateY(-10px)';
                subtitle.style.filter = 'blur(2px) brightness(200%)';
                subtitle.style.opacity = '0.7';
                
                setTimeout(() => {
                    subtitle.style.transform = 'scale(1.02)';
                    subtitle.style.filter = 'blur(0)';
                    subtitle.style.opacity = '1';
                }, 200);
                
                setTimeout(() => {
                    subtitle.style.transform = '';
                }, 400);
                
                const subtleVariations = ['whisper', 'breath', 'subtle', 'chromatic'];
                setTimeout(() => {
                    const variation1 = subtleVariations[Math.floor(Math.random() * subtleVariations.length)];
                    applyAmbientGlitch(subtitle, variation1);
                }, 300);
                
                setTimeout(() => {
                    const variation2 = subtleVariations[Math.floor(Math.random() * subtleVariations.length)];
                    applyAmbientGlitch(subtitle, variation2);
                }, 600);
                
            }, 400); 
        }
    }

    function welcomeSequence() {
        loadHeaderWithGlitch();
        
        addHeaderInteractiveEffects();
        
        const sections = document.querySelectorAll('.section');
        sections.forEach(section => {
            sectionObserver.observe(section);
        });
    }
    
    function addHeaderInteractiveEffects() {
        const headerTitles = document.querySelectorAll('.header .glitch-text');
        const subtitle = document.querySelector('.header .subtitle');
        
        headerTitles.forEach((title, index) => {
            title.addEventListener('mouseenter', function() {
                const glitchVariations = ['chromatic', 'colorSplit', 'digitalNoise', 'staticFlicker', 'fragmentShift'];
                const randomVariation = glitchVariations[Math.floor(Math.random() * glitchVariations.length)];
                
                this.style.transform = 'scale(1.03)';
                this.style.filter = 'brightness(120%) saturate(150%)';
                
                applyRandomGlitchVariation(this, randomVariation, 250);
                
                this.style.textShadow = '0 0 10px var(--color-primary-pink), 0 0 20px var(--color-primary-cyan)';
                
                setTimeout(() => {
                    this.style.transform = '';
                    this.style.filter = '';
                    this.style.textShadow = '';
                }, 300);
            });
            
            title.addEventListener('mouseleave', function() {
                const exitVariations = ['whisper', 'breath'];
                const variation = exitVariations[Math.floor(Math.random() * exitVariations.length)];
                applyAmbientGlitch(this, variation);
            });
            
            title.addEventListener('click', function(e) {
                e.preventDefault();
                
                const intenseVariations = ['dataCorruption', 'fragmentShift', 'matrixGlitch', 'staticFlicker'];
                
                this.style.transform = 'scale(0.95) rotate(1deg)';
                this.style.filter = 'contrast(200%) saturate(300%) hue-rotate(180deg)';
                
                setTimeout(() => {
                    this.style.transform = 'scale(1.08) rotate(-0.5deg)';
                    this.style.filter = 'contrast(150%) saturate(200%) brightness(150%)';
                }, 100);
                
                setTimeout(() => {
                    this.style.transform = '';
                    this.style.filter = '';
                }, 250);
                
                const variation1 = intenseVariations[Math.floor(Math.random() * intenseVariations.length)];
                const variation2 = intenseVariations[Math.floor(Math.random() * intenseVariations.length)];
                
                applyRandomGlitchVariation(this, variation1, 300);
                setTimeout(() => {
                    applyRandomGlitchVariation(this, variation2, 200);
                }, 150);
                
                createRippleEffect(this);
            });
            
            title.addEventListener('focus', function() {
                this.style.outline = '2px solid var(--color-primary-pink)';
                this.style.outlineOffset = '4px';
                applyRandomGlitchVariation(this, 'chromatic', 200);
            });
            
            title.addEventListener('blur', function() {
                this.style.outline = '';
                this.style.outlineOffset = '';
            });
            
            title.addEventListener('dblclick', function() {
                applySpectacularGlitchSequence(this);
            });
        });
        
        if (subtitle) {
            subtitle.addEventListener('mouseenter', function() {
                const subtleVariations = ['whisper', 'breath', 'subtle', 'chromatic'];
                const variation = subtleVariations[Math.floor(Math.random() * subtleVariations.length)];
                
                this.style.transform = 'scale(1.02)';
                this.style.filter = 'brightness(110%)';
                
                applyAmbientGlitch(this, variation);
                
                setTimeout(() => {
                    this.style.transform = '';
                    this.style.filter = '';
                }, 200);
            });
            
            subtitle.addEventListener('click', function() {
                applyTypewriterEffect(this);
            });
        }
    }
    
    function createRippleEffect(element) {
        const ripple = document.createElement('div');
        ripple.style.cssText = `
            position: absolute;
            border-radius: 50%;
            background: radial-gradient(circle, rgba(255, 0, 128, 0.6) 0%, rgba(0, 255, 255, 0.4) 50%, transparent 100%);
            transform: scale(0);
            animation: ripple 0.6s linear;
            pointer-events: none;
            width: 100px;
            height: 100px;
            left: 50%;
            top: 50%;
            margin-left: -50px;
            margin-top: -50px;
            z-index: -1;
        `;
        
        element.style.position = 'relative';
        element.appendChild(ripple);
        
        const style = document.createElement('style');
        style.textContent = `
            @keyframes ripple {
                to {
                    transform: scale(4);
                    opacity: 0;
                }
            }
        `;
        document.head.appendChild(style);
        
        setTimeout(() => {
            ripple.remove();
            style.remove();
        }, 600);
    }
    
    function applySpectacularGlitchSequence(element) {
        const allVariations = [
            'dataCorruption', 'chromatic', 'digitalNoise', 'scanlines',
            'fragmentShift', 'colorSplit', 'staticFlicker', 'matrixGlitch'
        ];
        
        let delay = 0;
        allVariations.forEach((variation, index) => {
            setTimeout(() => {
                applyRandomGlitchVariation(element, variation, 150);
            }, delay);
            delay += 100;
        });
        
        setTimeout(() => {
            element.style.transform = 'scale(1.2) rotate(360deg)';
            element.style.filter = 'hue-rotate(360deg) saturate(300%) brightness(200%)';
            element.style.textShadow = '0 0 30px var(--color-primary-pink), 0 0 60px var(--color-primary-cyan), 0 0 90px var(--color-accent-yellow)';
            
            setTimeout(() => {
                element.style.transform = '';
                element.style.filter = '';
                element.style.textShadow = '';
            }, 500);
        }, delay);
    }
    
    function applyTypewriterEffect(element) {
        const originalText = element.textContent;
        const chars = originalText.split('');
        element.textContent = '';
        
        let index = 0;
        const typeInterval = setInterval(() => {
            if (index < chars.length) {
                element.textContent += chars[index];
                
                if (Math.random() < 0.1) {
                    const glitchChar = '█▓▒░'[Math.floor(Math.random() * 4)];
                    element.textContent = element.textContent.slice(0, -1) + glitchChar;
                    setTimeout(() => {
                        element.textContent = element.textContent.slice(0, -1) + chars[index];
                    }, 50);
                }
                
                index++;
            } else {
                clearInterval(typeInterval);
            }
        }, 30);
    }
    
    welcomeSequence();
    initializeTitleEffects();
    initializeBackgroundEffects();
    document.addEventListener('error', function(e) {
        if (e.target.tagName === 'IMG' && e.target.classList.contains('team-member-image')) {
            const container = e.target.closest('.team-member-image-container');
            if (container) {
                createFallbackAvatar(container, e.target);
            }
        }
    }, true);
    
    function createFallbackAvatar(container, failedImg) {
        failedImg.remove();
        
        const altText = failedImg.alt || failedImg.title || 'Team Member';
        const initials = getInitials(altText);
        
        const fallback = document.createElement('div');
        fallback.className = 'team-member-avatar-fallback';
        fallback.setAttribute('data-initials', initials);
        
        container.appendChild(fallback);
        
        setTimeout(() => {
            fallback.classList.add('loaded');
        }, 100);
    }
    
    function getInitials(name) {
        return name
            .split(' ')
            .map(word => word.charAt(0).toUpperCase())
            .slice(0, 2)
            .join('');
    }
    
    function setupTeamCards() {
        const teamCards = document.querySelectorAll('.team-card');
        
        teamCards.forEach(card => {
            const imageContainer = card.querySelector('.team-member-image-container');
            const image = card.querySelector('.team-member-image');
            const fallback = card.querySelector('.team-member-avatar-fallback');
            
            if (image) {
                image.addEventListener('load', function() {
                    this.classList.add('loaded');
                });
                
                image.addEventListener('error', function() {
                    createFallbackAvatar(imageContainer, this);
                });
                
                if (image.complete && image.naturalHeight !== 0) {
                    image.classList.add('loaded');
                }
            }
            
            if (fallback) {
                fallback.classList.add('loaded');
            }
        });
    }
    
    setTimeout(setupTeamCards, 100);
    
}
