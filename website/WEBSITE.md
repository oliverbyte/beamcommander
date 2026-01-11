# BeamCommander Website

This directory contains the Jekyll-based website for BeamCommander that automatically deploys to GitHub Pages when changes are pushed to the main branch.

## Website Structure

- `_config.yml` - Jekyll configuration
- `_layouts/` - Page layouts
- `_pages/` - Content pages (features, installation, docs, faq)
- `assets/` - CSS, JavaScript, and other static assets
- `img/` - Images and GIFs
- `index.md` - Homepage
- `Gemfile` - Ruby dependencies
- `robots.txt` - Search engine crawler directives

## Local Development

To run the website locally:

### Option 1: Using Ruby/Jekyll (Recommended)
```bash
cd website

# Install dependencies (first time only)
bundle install

# Serve the site locally
bundle exec jekyll serve

# Visit http://localhost:4000/BeamCommander
```

### Option 2: Using Docker
```bash
cd website

docker run --rm \
  --volume="$PWD:/srv/jekyll" \
  --publish 4000:4000 \
  jekyll/jekyll:4.3 \
  jekyll serve --watch --force_polling

# Visit http://localhost:4000/BeamCommander
```

## Deployment

The website automatically deploys to GitHub Pages when changes are pushed to the `main` branch via GitHub Actions workflow (`.github/workflows/jekyll-gh-pages.yml`).

The site will be available at: `https://oliverbyte.github.io/BeamCommander/`

## SEO Optimization

The site includes:
- SEO-optimized meta tags via `jekyll-seo-tag`
- Automatic sitemap generation via `jekyll-sitemap`
- RSS feed via `jekyll-feed`
- Semantic HTML structure
- Open Graph and Twitter Card meta tags
- robots.txt for search engine crawling

## Design

The website uses:
- Modern, gradient-based design
- Responsive layout for mobile and desktop
- Clean typography with Inter font
- Smooth animations and transitions
- Accessible color contrast
- SEO-friendly structure

## Content Updates

To update content:
1. Edit the relevant Markdown file in `_pages/` or `index.md`
2. Commit and push to the `main` branch
3. GitHub Actions will automatically rebuild and deploy the site

## Image Assets

Place demonstration GIFs and images in the `img/` directory:
- `demo.gif` - Main demonstration GIF
- `live-demo.gif` - Live performance demonstration
- Other assets as needed

## License

This website is part of the BeamCommander project.
