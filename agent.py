#!/usr/bin/env python3
"""
Web-search agent that researches design practices and writes design-practices.md.

Usage:
    python agent.py

The script searches the web for authoritative resources on web and UI/UX design
practices, fetches and summarises their key points, then writes a structured
Markdown file called ``design-practices.md`` in the current directory.
"""

import re
import textwrap
import time
from urllib.parse import quote_plus, urljoin, urlparse

import requests
from bs4 import BeautifulSoup

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

HEADERS = {
    "User-Agent": (
        "Mozilla/5.0 (compatible; design-practices-agent/1.0; "
        "+https://github.com/mlmyerson/mandelbrot-viewer)"
    )
}
TIMEOUT = 15  # seconds per request

# Search queries → section headings in the output document
QUERIES = [
    ("web design best practices", "Web Design Best Practices"),
    ("UI UX design principles", "UI / UX Design Principles"),
    ("responsive design best practices", "Responsive Design"),
    ("web accessibility design guidelines", "Accessibility"),
    ("web performance optimization design", "Performance & Optimisation"),
    ("color theory web design", "Colour Theory & Visual Hierarchy"),
    ("typography web design best practices", "Typography"),
]

OUTPUT_FILE = "design-practices.md"
MAX_RESULTS_PER_QUERY = 3  # number of pages to read per query
MAX_CHARS_PER_PAGE = 4000   # characters of body text to extract per page


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def duckduckgo_search(query: str, max_results: int = MAX_RESULTS_PER_QUERY) -> list[dict]:
    """
    Use DuckDuckGo HTML search (no API key required) to find results for
    *query* and return a list of ``{"title": …, "url": …}`` dicts.
    """
    url = f"https://html.duckduckgo.com/html/?q={quote_plus(query)}"
    results = []
    try:
        resp = requests.get(url, headers=HEADERS, timeout=TIMEOUT)
        resp.raise_for_status()
        soup = BeautifulSoup(resp.text, "html.parser")
        for a in soup.select("a.result__a"):
            href = a.get("href", "")
            title = a.get_text(strip=True)
            # DuckDuckGo wraps URLs – resolve the real destination
            parsed = urlparse(href)
            if parsed.scheme in ("http", "https") and parsed.netloc:
                results.append({"title": title, "url": href})
            if len(results) >= max_results:
                break
    except requests.RequestException as exc:
        print(f"  [search error] {type(exc).__name__}: {exc}")
    except Exception as exc:  # noqa: BLE001
        print(f"  [search error] unexpected {type(exc).__name__}: {exc}")
    return results


def fetch_page_text(url: str, max_chars: int = MAX_CHARS_PER_PAGE) -> str:
    """
    Fetch *url* and return a plain-text extract of its main content,
    capped at *max_chars* characters.
    """
    try:
        resp = requests.get(url, headers=HEADERS, timeout=TIMEOUT, allow_redirects=True)
        resp.raise_for_status()
        soup = BeautifulSoup(resp.text, "html.parser")

        # Remove navigation, scripts, styles and ads
        for tag in soup(["script", "style", "nav", "header", "footer",
                          "aside", "form", "noscript", "iframe"]):
            tag.decompose()

        # Prefer <article> or <main>, fall back to <body>
        container = soup.find("article") or soup.find("main") or soup.find("body")
        if container is None:
            return ""

        text = container.get_text(separator="\n")
        # Collapse runs of blank lines
        text = re.sub(r"\n{3,}", "\n\n", text).strip()
        return text[:max_chars]
    except requests.RequestException as exc:
        print(f"  [fetch error] {type(exc).__name__} fetching {url}: {exc}")
        return ""
    except Exception as exc:  # noqa: BLE001
        print(f"  [fetch error] unexpected {type(exc).__name__} fetching {url}: {exc}")
        return ""


def extract_bullet_points(text: str, max_bullets: int = 10) -> list[str]:
    """
    Heuristically extract short, meaningful sentences from *text* that read
    like tips or principles (contain action verbs, design keywords, etc.).
    Returns up to *max_bullets* items.
    """
    design_keywords = re.compile(
        r"\b(design|layout|colour|color|font|typography|contrast|spacing|"
        r"padding|margin|responsive|mobile|accessib|usab|perform|load|image|"
        r"button|navigation|user|visual|hierarchy|whitespace|grid|align|"
        r"consistent|feedback|error|readab|legib|icon|cta|call.to.action|"
        r"balance|simplif|minimal|clean|intuit)\b",
        re.IGNORECASE,
    )

    bullets = []
    for line in text.splitlines():
        line = line.strip()
        # Skip very short or very long lines
        if len(line) < 30 or len(line) > 200:
            continue
        # Skip lines that look like navigation or boilerplate
        if re.match(r"^(menu|home|about|contact|skip|©|cookie|privacy)", line, re.I):
            continue
        if design_keywords.search(line):
            # Clean up leading list markers
            line = re.sub(r"^[-•*·]\s*", "", line)
            line = re.sub(r"^\d+[.)]\s*", "", line)
            if line and line not in bullets:
                bullets.append(line)
        if len(bullets) >= max_bullets:
            break
    return bullets


# ---------------------------------------------------------------------------
# Main agent logic
# ---------------------------------------------------------------------------

def run_agent() -> None:
    print("=== Design-Practices Agent ===\n")

    sections: list[tuple[str, list[tuple[str, str, list[str]]]]] = []
    # structure: [(section_heading, [(page_title, page_url, [bullet, …]), …]), …]

    for query, section_heading in QUERIES:
        print(f"[search] {query}")
        results = duckduckgo_search(query)
        if not results:
            print("  No results found, skipping.")
            sections.append((section_heading, []))
            continue

        page_summaries = []
        for result in results:
            title = result["title"]
            url = result["url"]
            print(f"  [fetch]  {url}")
            text = fetch_page_text(url)
            bullets = extract_bullet_points(text)
            if bullets:
                page_summaries.append((title, url, bullets))
            time.sleep(0.5)  # be polite

        sections.append((section_heading, page_summaries))

    # -----------------------------------------------------------------------
    # Build the Markdown document
    # -----------------------------------------------------------------------
    lines: list[str] = [
        "# Design Practices",
        "",
        "> This document was generated automatically by `agent.py`, which searches",
        "> the web for authoritative resources on design best practices and",
        "> synthesises their key points into this reference guide.",
        "",
        "---",
        "",
        "## Table of Contents",
        "",
    ]
    for i, (heading, _) in enumerate(sections, start=1):
        anchor = re.sub(r"[^\w\s-]", "", heading.lower())
        anchor = re.sub(r"[\s]+", "-", anchor.strip())
        lines.append(f"{i}. [{heading}](#{anchor})")
    lines += ["", "---", ""]

    for heading, page_summaries in sections:
        lines += [f"## {heading}", ""]
        if not page_summaries:
            lines += ["_No content could be retrieved for this topic._", ""]
            continue
        for page_title, page_url, bullets in page_summaries:
            lines += [f"### {page_title}", "", f"_Source: <{page_url}>_", ""]
            for bullet in bullets:
                # Wrap long bullets for readability
                wrapped = textwrap.fill(bullet, width=100, subsequent_indent="  ")
                lines.append(f"- {wrapped}")
            lines.append("")

    lines += [
        "---",
        "",
        f"_Generated by the design-practices agent — {time.strftime('%Y-%m-%d %H:%M UTC', time.gmtime())}_",
        "",
    ]

    markdown = "\n".join(lines)

    with open(OUTPUT_FILE, "w", encoding="utf-8") as fh:
        fh.write(markdown)

    print(f"\n✓ Wrote {OUTPUT_FILE} ({len(markdown):,} characters).")


if __name__ == "__main__":
    run_agent()
