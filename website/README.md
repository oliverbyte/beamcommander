# BeamCommander Website - Developer Guide

This directory contains the Jekyll-based website for BeamCommander, deployed to GitHub Pages.

## 🚀 Quick Start

### Local Development

The easiest way to preview the website locally:

1. **Build the site** (only needed after content changes):
   ```bash
   docker run --rm --volume="$PWD:/srv/jekyll" jekyll/jekyll:latest jekyll build
   ```

2. **Start the preview server**:
   ```bash
   cd _site && python3 -m http.server 4000
   ```

3. **Open in browser**: http://localhost:4000/

That's it! No Ruby installation needed on your machine.

## 📁 Project Structure

```
website/
├── _config.yml           # Jekyll configuration
├── _layouts/
│   └── default.html      # Main HTML layout template
├── _pages/               # Static pages
│   ├── features.md
│   ├── installation.md
│   ├── docs.md
│   └── faq.md
├── assets/
│   ├── css/
│   │   └── style.css     # All styles
│   └── js/
│       └── main.js       # JavaScript functionality
├── img/                  # Images and demos
├── index.md              # Homepage
├── Gemfile               # Ruby dependencies
└── _site/                # Built site (generated, not in git)
```

## 🛠️ Making Changes

### Content Updates

1. **Edit Markdown files** in `_pages/` or `index.md`
2. **Rebuild the site** (see Quick Start step 1)
3. **Refresh browser** to see changes

### Style Changes

1. **Edit** `assets/css/style.css`
2. **Rebuild** the site
3. **Hard refresh** browser (Cmd+Shift+R on Mac)

### Layout Changes

1. **Edit** `_layouts/default.html`
2. **Rebuild** the site
3. **Refresh** browser

## 🎨 Design Guidelines

### Color Palette
- Primary: `#8b5cf6` (Purple)
- Primary Light: `#a78bfa`
- Background: `#0f172a` (Dark blue)
- Text Dark: `#1e293b`

### Typography
- Font: Inter (Google Fonts)
- Weights: 300, 400, 500, 600, 700

### Responsive Breakpoints
- Desktop: 1024px+
- Tablet: 768px - 1023px
- Mobile: < 768px

## 📝 Content Best Practices

### SEO Guidelines
1. **H1 tags** must include "BeamCommander" keyword
2. **Meta descriptions** should be 150-160 characters
3. **Alt text** for all images
4. **Internal linking** between pages

### Writing Style
- Clear, concise language
- Active voice
- Focus on user benefits
- Technical accuracy

## 🚢 Deployment

### Automatic Deployment
The site automatically deploys to GitHub Pages when changes are pushed to the `main` branch.

**GitHub Actions Workflow**: `.github/workflows/jekyll-gh-pages.yml`

### Manual Deployment
Not needed - the workflow handles everything automatically:
1. Installs Ruby and dependencies
2. Builds site with correct `baseurl`
3. Deploys to GitHub Pages

## ⚙️ Configuration

### Local Development vs Production

The `_config.yml` uses `baseurl: ""` for local development. The GitHub Actions workflow automatically sets the correct baseurl for production:

```yaml
# _config.yml (for local dev)
baseurl: ""  # Empty for localhost

# GitHub Actions (for production)
baseurl: "/BeamCommander"  # Auto-set by workflow
```

### Key Configuration Values
- `title`: Site title (appears in browser tab)
- `description`: Meta description for SEO
- `repository`: GitHub repo (oliverbyte/BeamCommander)
- `url`: Base URL (https://oliverbyte.github.io)

## 🔧 Troubleshooting

### CSS not loading?
- Check that you rebuilt the site after CSS changes
- Verify `_site/assets/css/style.css` exists
- Hard refresh browser (Cmd+Shift+R)

### Changes not appearing?
- Make sure you rebuilt: `docker run --rm --volume="$PWD:/srv/jekyll" jekyll/jekyll:latest jekyll build`
- Check for build errors in terminal output
- Restart the Python server

### Port 4000 already in use?
```bash
# Find and kill the process
lsof -ti:4000 | xargs kill -9

# Or use a different port
python3 -m http.server 4001
```

## 📦 Dependencies

### Jekyll Plugins
- `jekyll-seo-tag` - SEO meta tags
- `jekyll-sitemap` - XML sitemap generation
- `jekyll-feed` - RSS feed generation

### Frontend Dependencies
- **No build tools needed!** Plain CSS and vanilla JavaScript
- Google Fonts (Inter) - loaded via CDN
- No npm, webpack, or bundlers required

## 🤝 Contributing

### Before Submitting Changes
1. Test locally using the preview server
2. Check responsive design (resize browser window)
3. Verify all links work
4. Run a quick SEO check (H1, meta description, alt texts)
5. Commit with clear message

### Commit Message Format
```
type: brief description

- Detail 1
- Detail 2
```

Types: `content`, `style`, `fix`, `feature`, `docs`

Example:
```
content: add laser safety guidelines to FAQ

- Added safety section
- Updated installation warnings
- Added link from installation page
```

## 📚 Useful Commands

```bash
# Build site
docker run --rm --volume="$PWD:/srv/jekyll" jekyll/jekyll:latest jekyll build

# Build with specific config
docker run --rm --volume="$PWD:/srv/jekyll" jekyll/jekyll:latest jekyll build --config _config.yml

# Start preview server
cd _site && python3 -m http.server 4000

# Kill server
lsof -ti:4000 | xargs kill -9

# Clean build (remove _site)
rm -rf _site
```

## 🌐 Live URLs

- **Production**: https://oliverbyte.github.io/BeamCommander/
- **Repository**: https://github.com/oliverbyte/BeamCommander
- **Local Preview**: http://localhost:4000/

## 📖 Learn More

- [Jekyll Documentation](https://jekyllrb.com/docs/)
- [GitHub Pages Documentation](https://docs.github.com/en/pages)
- [Markdown Guide](https://www.markdownguide.org/)

---

**Questions?** Open an issue on GitHub or check the main project README.
