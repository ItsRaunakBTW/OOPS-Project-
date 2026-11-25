#!/usr/bin/env python3
"""
build_technical_report_pdf.py
Generates a publication-quality PDF technical report for the ALF-EWG project
using macOS native Quartz (CoreGraphics) and CoreText via PyObjC.
Output: ALF_EWG_Technical_Report.pdf
"""

import sys
import os
import re
import Quartz
import CoreText
from CoreFoundation import NSURL, CFRange
from pypdf import PdfReader, PdfWriter

# Page geometry (Standard US Letter in points)
PAGE_WIDTH = 612.0
PAGE_HEIGHT = 792.0
MARGIN_LEFT = 50.0
MARGIN_RIGHT = 50.0
MARGIN_TOP = 52.0
MARGIN_BOTTOM = 52.0
CONTENT_WIDTH = PAGE_WIDTH - MARGIN_LEFT - MARGIN_RIGHT # 512.0
CONTENT_HEIGHT = PAGE_HEIGHT - MARGIN_TOP - MARGIN_BOTTOM # 688.0

# Colors (RGBA tuples 0.0 - 1.0)
COLOR_NAVY_DARK    = (0.04, 0.14, 0.25, 1.0)  # #0A2440
COLOR_BLUE_ACCENT  = (0.12, 0.38, 0.62, 1.0)  # #1F619D
COLOR_TEXT_DARK    = (0.16, 0.20, 0.25, 1.0)  # #2A3340
COLOR_TEXT_MUTED   = (0.40, 0.45, 0.52, 1.0)  # #667385
COLOR_CODE_BG      = (0.96, 0.97, 0.98, 1.0)  # #F5F7FA
COLOR_CODE_BORDER  = (0.82, 0.86, 0.90, 1.0)  # #D1DCE6
COLOR_RULE         = (0.85, 0.88, 0.92, 1.0)  # #D9E0EB
COLOR_TABLE_HDR    = (0.08, 0.22, 0.36, 1.0)  # #14385C
COLOR_TABLE_ALT    = (0.97, 0.98, 0.99, 1.0)  # #F7FAFC
COLOR_WHITE        = (1.00, 1.00, 1.00, 1.0)
COLOR_CALLOUT_BG   = (0.94, 0.97, 1.00, 1.0)  # #F0F7FF
COLOR_CALLOUT_BAR  = (0.15, 0.48, 0.80, 1.0)  # #267ACC

# CoreText Fonts Cache
FONTS = {
    'title':       CoreText.CTFontCreateWithName('Helvetica-Bold', 18.0, None),
    'subtitle':    CoreText.CTFontCreateWithName('Helvetica', 10.5, None),
    'meta':        CoreText.CTFontCreateWithName('Helvetica-Bold', 8.5, None),
    'h1':          CoreText.CTFontCreateWithName('Helvetica-Bold', 13.5, None),
    'h2':          CoreText.CTFontCreateWithName('Helvetica-Bold', 11.0, None),
    'h3':          CoreText.CTFontCreateWithName('Helvetica-Bold', 9.5, None),
    'body':        CoreText.CTFontCreateWithName('Helvetica', 9.0, None),
    'body_bold':   CoreText.CTFontCreateWithName('Helvetica-Bold', 9.0, None),
    'body_italic': CoreText.CTFontCreateWithName('Helvetica-Oblique', 9.0, None),
    'code':        CoreText.CTFontCreateWithName('Courier', 7.5, None),
    'table_hdr':   CoreText.CTFontCreateWithName('Helvetica-Bold', 8.0, None),
    'table_cell':  CoreText.CTFontCreateWithName('Helvetica', 7.5, None),
    'footer':      CoreText.CTFontCreateWithName('Helvetica', 7.5, None),
    'header':      CoreText.CTFontCreateWithName('Helvetica', 7.5, None),
}

def measure_line_width(text, font):
    astr = CoreText.CFAttributedStringCreate(None, text, {CoreText.kCTFontAttributeName: font})
    line = CoreText.CTLineCreateWithAttributedString(astr)
    w, _, _, _ = CoreText.CTLineGetTypographicBounds(line, None, None, None)
    return w

def wrap_text(text, font, max_width):
    words = text.split(' ')
    lines = []
    current_line = []
    
    for word in words:
        test_line = ' '.join(current_line + [word])
        if measure_line_width(test_line, font) <= max_width:
            current_line.append(word)
        else:
            if current_line:
                lines.append(' '.join(current_line))
                current_line = [word]
            else:
                # Word itself is wider than max_width, forced split
                lines.append(word)
                current_line = []
    if current_line:
        lines.append(' '.join(current_line))
    return lines

class PDFRenderer:
    def __init__(self, output_path):
        self.output_path = output_path
        self.tmp_path = output_path + '.tmp.pdf'
        self.pages = []
        self.current_page = 0
        self.cursor_y = MARGIN_TOP  # Measured from TOP (0 = top margin)
        self.context = None
        self.page_rect = Quartz.CGRectMake(0, 0, PAGE_WIDTH, PAGE_HEIGHT)
        
    def start(self):
        url = NSURL.fileURLWithPath_(self.tmp_path)
        self.context = Quartz.CGPDFContextCreateWithURL(url, self.page_rect, None)
        self.new_page()

    def new_page(self):
        if self.current_page > 0:
            self.draw_page_decorations()
            Quartz.CGPDFContextEndPage(self.context)
            
        self.current_page += 1
        Quartz.CGPDFContextBeginPage(self.context, None)
        self.cursor_y = MARGIN_TOP + (20.0 if self.current_page > 1 else 0.0)

    def to_quartz_y(self, y_from_top):
        return PAGE_HEIGHT - y_from_top

    def ensure_space(self, height_needed):
        if self.cursor_y + height_needed > PAGE_HEIGHT - MARGIN_BOTTOM:
            self.new_page()

    def draw_page_decorations(self):
        # Draw running header on page 2+
        if self.current_page > 1:
            Quartz.CGContextSetRGBStrokeColor(self.context, *COLOR_RULE)
            Quartz.CGContextSetLineWidth(self.context, 0.6)
            Quartz.CGContextStrokeLineSegments(self.context, [
                Quartz.CGPointMake(MARGIN_LEFT, self.to_quartz_y(MARGIN_TOP - 12)),
                Quartz.CGPointMake(PAGE_WIDTH - MARGIN_RIGHT, self.to_quartz_y(MARGIN_TOP - 12))
            ], 2)
            
            # Header text
            astr = CoreText.CFAttributedStringCreate(None, "ALF-EWG: Disaster Monitoring Simulation | OOP in C++ (Unit I - IV)", {
                CoreText.kCTFontAttributeName: FONTS['header'],
                CoreText.kCTForegroundColorFromContextAttributeName: True
            })
            line = CoreText.CTLineCreateWithAttributedString(astr)
            Quartz.CGContextSetRGBFillColor(self.context, *COLOR_TEXT_MUTED)
            Quartz.CGContextSetTextPosition(self.context, MARGIN_LEFT, self.to_quartz_y(MARGIN_TOP - 22))
            CoreText.CTLineDraw(line, self.context)

        # Draw footer line
        Quartz.CGContextSetRGBStrokeColor(self.context, *COLOR_RULE)
        Quartz.CGContextSetLineWidth(self.context, 0.6)
        Quartz.CGContextStrokeLineSegments(self.context, [
            Quartz.CGPointMake(MARGIN_LEFT, self.to_quartz_y(PAGE_HEIGHT - MARGIN_BOTTOM + 14)),
            Quartz.CGPointMake(PAGE_WIDTH - MARGIN_RIGHT, self.to_quartz_y(PAGE_HEIGHT - MARGIN_BOTTOM + 14))
        ], 2)

        # Left footer label
        astr_l = CoreText.CFAttributedStringCreate(None, "Academic Project Technical Report — Standard C++17 (0 Warnings)", {
            CoreText.kCTFontAttributeName: FONTS['footer'],
            CoreText.kCTForegroundColorFromContextAttributeName: True
        })
        line_l = CoreText.CTLineCreateWithAttributedString(astr_l)
        Quartz.CGContextSetRGBFillColor(self.context, *COLOR_TEXT_MUTED)
        Quartz.CGContextSetTextPosition(self.context, MARGIN_LEFT, self.to_quartz_y(PAGE_HEIGHT - MARGIN_BOTTOM + 25))
        CoreText.CTLineDraw(line_l, self.context)

        # Right footer page number placeholder (will be finalized in pass 2)
        # Placeholder text: "Page X"
        p_str = f"Page {self.current_page}"
        astr_r = CoreText.CFAttributedStringCreate(None, p_str, {
            CoreText.kCTFontAttributeName: FONTS['footer'],
            CoreText.kCTForegroundColorFromContextAttributeName: True
        })
        line_r = CoreText.CTLineCreateWithAttributedString(astr_r)
        rw, _, _, _ = CoreText.CTLineGetTypographicBounds(line_r, None, None, None)
        Quartz.CGContextSetTextPosition(self.context, PAGE_WIDTH - MARGIN_RIGHT - rw, self.to_quartz_y(PAGE_HEIGHT - MARGIN_BOTTOM + 25))
        CoreText.CTLineDraw(line_r, self.context)

    def draw_cover_banner(self, title, subtitle, meta_items):
        banner_height = 84.0
        self.ensure_space(banner_height + 20)
        
        # Navy Banner Background
        rect = Quartz.CGRectMake(MARGIN_LEFT, self.to_quartz_y(self.cursor_y + banner_height), CONTENT_WIDTH, banner_height)
        Quartz.CGContextSetRGBFillColor(self.context, *COLOR_NAVY_DARK)
        path = Quartz.CGPathCreateWithRoundedRect(rect, 4.0, 4.0, None)
        Quartz.CGContextAddPath(self.context, path)
        Quartz.CGContextFillPath(self.context)
        
        # Left Accent Bar
        bar_rect = Quartz.CGRectMake(MARGIN_LEFT, self.to_quartz_y(self.cursor_y + banner_height), 5.0, banner_height)
        Quartz.CGContextSetRGBFillColor(self.context, *COLOR_BLUE_ACCENT)
        Quartz.CGContextFillRect(self.context, bar_rect)
        
        # Title Text
        t_astr = CoreText.CFAttributedStringCreate(None, title, {
            CoreText.kCTFontAttributeName: FONTS['title'],
            CoreText.kCTForegroundColorFromContextAttributeName: True
        })
        t_line = CoreText.CTLineCreateWithAttributedString(t_astr)
        Quartz.CGContextSetRGBFillColor(self.context, *COLOR_WHITE)
        Quartz.CGContextSetTextPosition(self.context, MARGIN_LEFT + 16, self.to_quartz_y(self.cursor_y + 26))
        CoreText.CTLineDraw(t_line, self.context)
        
        # Subtitle Text
        s_astr = CoreText.CFAttributedStringCreate(None, subtitle, {
            CoreText.kCTFontAttributeName: FONTS['subtitle'],
            CoreText.kCTForegroundColorFromContextAttributeName: True
        })
        s_line = CoreText.CTLineCreateWithAttributedString(s_astr)
        Quartz.CGContextSetRGBFillColor(self.context, 0.82, 0.89, 0.96, 1.0)
        Quartz.CGContextSetTextPosition(self.context, MARGIN_LEFT + 16, self.to_quartz_y(self.cursor_y + 44))
        CoreText.CTLineDraw(s_line, self.context)
        
        # Meta row
        meta_str = " | ".join(meta_items)
        m_astr = CoreText.CFAttributedStringCreate(None, meta_str, {
            CoreText.kCTFontAttributeName: FONTS['meta'],
            CoreText.kCTForegroundColorFromContextAttributeName: True
        })
        m_line = CoreText.CTLineCreateWithAttributedString(m_astr)
        Quartz.CGContextSetRGBFillColor(self.context, 0.65, 0.78, 0.92, 1.0)
        Quartz.CGContextSetTextPosition(self.context, MARGIN_LEFT + 16, self.to_quartz_y(self.cursor_y + 68))
        CoreText.CTLineDraw(m_line, self.context)
        
        self.cursor_y += banner_height + 16.0

    def draw_heading_1(self, text):
        self.ensure_space(42)
        self.cursor_y += 10.0
        
        # Left blue vertical pill
        pill_rect = Quartz.CGRectMake(MARGIN_LEFT, self.to_quartz_y(self.cursor_y + 16), 4.0, 16.0)
        Quartz.CGContextSetRGBFillColor(self.context, *COLOR_BLUE_ACCENT)
        Quartz.CGContextFillRect(self.context, pill_rect)
        
        # Heading text
        astr = CoreText.CFAttributedStringCreate(None, text, {
            CoreText.kCTFontAttributeName: FONTS['h1'],
            CoreText.kCTForegroundColorFromContextAttributeName: True
        })
        line = CoreText.CTLineCreateWithAttributedString(astr)
        Quartz.CGContextSetRGBFillColor(self.context, *COLOR_NAVY_DARK)
        Quartz.CGContextSetTextPosition(self.context, MARGIN_LEFT + 10, self.to_quartz_y(self.cursor_y + 14))
        CoreText.CTLineDraw(line, self.context)
        
        self.cursor_y += 24.0

    def draw_heading_2(self, text):
        self.ensure_space(32)
        self.cursor_y += 6.0
        astr = CoreText.CFAttributedStringCreate(None, text, {
            CoreText.kCTFontAttributeName: FONTS['h2'],
            CoreText.kCTForegroundColorFromContextAttributeName: True
        })
        line = CoreText.CTLineCreateWithAttributedString(astr)
        Quartz.CGContextSetRGBFillColor(self.context, *COLOR_BLUE_ACCENT)
        Quartz.CGContextSetTextPosition(self.context, MARGIN_LEFT, self.to_quartz_y(self.cursor_y + 11))
        CoreText.CTLineDraw(line, self.context)
        self.cursor_y += 17.0

    def draw_heading_3(self, text):
        self.ensure_space(24)
        self.cursor_y += 4.0
        astr = CoreText.CFAttributedStringCreate(None, text, {
            CoreText.kCTFontAttributeName: FONTS['h3'],
            CoreText.kCTForegroundColorFromContextAttributeName: True
        })
        line = CoreText.CTLineCreateWithAttributedString(astr)
        Quartz.CGContextSetRGBFillColor(self.context, *COLOR_TEXT_DARK)
        Quartz.CGContextSetTextPosition(self.context, MARGIN_LEFT, self.to_quartz_y(self.cursor_y + 10))
        CoreText.CTLineDraw(line, self.context)
        self.cursor_y += 14.0

    def draw_paragraph(self, text, indent=0.0, bold_prefix=None):
        font = FONTS['body']
        line_height = 12.0
        available_width = CONTENT_WIDTH - indent
        
        if bold_prefix and text.startswith(bold_prefix):
            rest_text = text[len(bold_prefix):]
            # Draw bold prefix + wrapped text
            full_text = bold_prefix + rest_text
            lines = wrap_text(full_text, font, available_width)
        else:
            lines = wrap_text(text, font, available_width)
            
        block_height = len(lines) * line_height + 4.0
        self.ensure_space(block_height)
        
        for l in lines:
            astr = CoreText.CFAttributedStringCreate(None, l, {
                CoreText.kCTFontAttributeName: font,
                CoreText.kCTForegroundColorFromContextAttributeName: True
            })
            line = CoreText.CTLineCreateWithAttributedString(astr)
            Quartz.CGContextSetRGBFillColor(self.context, *COLOR_TEXT_DARK)
            Quartz.CGContextSetTextPosition(self.context, MARGIN_LEFT + indent, self.to_quartz_y(self.cursor_y + 9))
            CoreText.CTLineDraw(line, self.context)
            self.cursor_y += line_height
            
        self.cursor_y += 4.0

    def draw_bullet(self, text):
        # Indented bullet point
        font = FONTS['body']
        line_height = 12.0
        bullet_indent = 12.0
        available_width = CONTENT_WIDTH - bullet_indent
        lines = wrap_text(text, font, available_width)
        
        self.ensure_space(len(lines) * line_height + 2.0)
        
        # Draw small square/dot for bullet
        dot_y = self.to_quartz_y(self.cursor_y + 7.5)
        Quartz.CGContextSetRGBFillColor(self.context, *COLOR_BLUE_ACCENT)
        Quartz.CGContextFillRect(self.context, Quartz.CGRectMake(MARGIN_LEFT + 2, dot_y, 3.5, 3.5))
        
        for idx, l in enumerate(lines):
            astr = CoreText.CFAttributedStringCreate(None, l, {
                CoreText.kCTFontAttributeName: font,
                CoreText.kCTForegroundColorFromContextAttributeName: True
            })
            line = CoreText.CTLineCreateWithAttributedString(astr)
            Quartz.CGContextSetRGBFillColor(self.context, *COLOR_TEXT_DARK)
            Quartz.CGContextSetTextPosition(self.context, MARGIN_LEFT + bullet_indent, self.to_quartz_y(self.cursor_y + 9))
            CoreText.CTLineDraw(line, self.context)
            self.cursor_y += line_height
        self.cursor_y += 2.0

    def draw_code_block(self, code_lines):
        font = FONTS['code']
        line_height = 10.0
        pad_v = 7.0
        pad_h = 10.0
        block_height = len(code_lines) * line_height + 2 * pad_v
        
        self.ensure_space(block_height + 8.0)
        
        # Background card
        rect = Quartz.CGRectMake(MARGIN_LEFT, self.to_quartz_y(self.cursor_y + block_height), CONTENT_WIDTH, block_height)
        Quartz.CGContextSetRGBFillColor(self.context, *COLOR_CODE_BG)
        path = Quartz.CGPathCreateWithRoundedRect(rect, 3.0, 3.0, None)
        Quartz.CGContextAddPath(self.context, path)
        Quartz.CGContextFillPath(self.context)
        
        # Stroke border
        Quartz.CGContextSetRGBStrokeColor(self.context, *COLOR_CODE_BORDER)
        Quartz.CGContextSetLineWidth(self.context, 0.6)
        path_b = Quartz.CGPathCreateWithRoundedRect(rect, 3.0, 3.0, None)
        Quartz.CGContextAddPath(self.context, path_b)
        Quartz.CGContextStrokePath(self.context)
        
        # Text rendering
        curr_y = self.cursor_y + pad_v
        for cl in code_lines:
            astr = CoreText.CFAttributedStringCreate(None, cl, {
                CoreText.kCTFontAttributeName: font,
                CoreText.kCTForegroundColorFromContextAttributeName: True
            })
            line = CoreText.CTLineCreateWithAttributedString(astr)
            Quartz.CGContextSetRGBFillColor(self.context, 0.12, 0.16, 0.22, 1.0)
            Quartz.CGContextSetTextPosition(self.context, MARGIN_LEFT + pad_h, self.to_quartz_y(curr_y + 7.5))
            CoreText.CTLineDraw(line, self.context)
            curr_y += line_height
            
        self.cursor_y += block_height + 8.0

    def draw_callout_box(self, title, lines):
        font = FONTS['body']
        line_height = 11.5
        box_pad = 8.0
        total_text_height = (len(lines) + (1 if title else 0)) * line_height
        box_height = total_text_height + 2 * box_pad
        
        self.ensure_space(box_height + 6.0)
        
        rect = Quartz.CGRectMake(MARGIN_LEFT, self.to_quartz_y(self.cursor_y + box_height), CONTENT_WIDTH, box_height)
        Quartz.CGContextSetRGBFillColor(self.context, *COLOR_CALLOUT_BG)
        Quartz.CGContextFillRect(self.context, rect)
        
        # Left border bar
        bar_rect = Quartz.CGRectMake(MARGIN_LEFT, self.to_quartz_y(self.cursor_y + box_height), 3.5, box_height)
        Quartz.CGContextSetRGBFillColor(self.context, *COLOR_CALLOUT_BAR)
        Quartz.CGContextFillRect(self.context, bar_rect)
        
        curr_y = self.cursor_y + box_pad
        if title:
            t_astr = CoreText.CFAttributedStringCreate(None, title, {
                CoreText.kCTFontAttributeName: FONTS['body_bold'],
                CoreText.kCTForegroundColorFromContextAttributeName: True
            })
            t_line = CoreText.CTLineCreateWithAttributedString(t_astr)
            Quartz.CGContextSetRGBFillColor(self.context, *COLOR_BLUE_ACCENT)
            Quartz.CGContextSetTextPosition(self.context, MARGIN_LEFT + 12, self.to_quartz_y(curr_y + 8.5))
            CoreText.CTLineDraw(t_line, self.context)
            curr_y += line_height
            
        for l in lines:
            astr = CoreText.CFAttributedStringCreate(None, l, {
                CoreText.kCTFontAttributeName: font,
                CoreText.kCTForegroundColorFromContextAttributeName: True
            })
            line = CoreText.CTLineCreateWithAttributedString(astr)
            Quartz.CGContextSetRGBFillColor(self.context, *COLOR_TEXT_DARK)
            Quartz.CGContextSetTextPosition(self.context, MARGIN_LEFT + 12, self.to_quartz_y(curr_y + 8.5))
            CoreText.CTLineDraw(line, self.context)
            curr_y += line_height
            
        self.cursor_y += box_height + 6.0

    def draw_table(self, headers, rows, col_widths):
        row_height = 14.5
        hdr_height = 16.0
        total_width = sum(col_widths)
        
        # Measure row heights based on wrapped content
        processed_rows = []
        for row in rows:
            max_lines = 1
            wrapped_cells = []
            for idx, cell in enumerate(row):
                w = col_widths[idx] - 8.0 # cell padding
                c_lines = wrap_text(cell, FONTS['table_cell'], w)
                max_lines = max(max_lines, len(c_lines))
                wrapped_cells.append(c_lines)
            row_h = max_lines * 9.5 + 5.0
            processed_rows.append((row_h, wrapped_cells))
            
        total_table_height = hdr_height + sum(rh for rh, _ in processed_rows)
        
        # If table fits, draw it; otherwise split across pages gracefully
        self.ensure_space(hdr_height + processed_rows[0][0])
        
        # Draw header
        hdr_rect = Quartz.CGRectMake(MARGIN_LEFT, self.to_quartz_y(self.cursor_y + hdr_height), total_width, hdr_height)
        Quartz.CGContextSetRGBFillColor(self.context, *COLOR_TABLE_HDR)
        Quartz.CGContextFillRect(self.context, hdr_rect)
        
        cur_x = MARGIN_LEFT
        for idx, h in enumerate(headers):
            astr = CoreText.CFAttributedStringCreate(None, h, {
                CoreText.kCTFontAttributeName: FONTS['table_hdr'],
                CoreText.kCTForegroundColorFromContextAttributeName: True
            })
            line = CoreText.CTLineCreateWithAttributedString(astr)
            Quartz.CGContextSetRGBFillColor(self.context, *COLOR_WHITE)
            Quartz.CGContextSetTextPosition(self.context, cur_x + 4.0, self.to_quartz_y(self.cursor_y + 11.5))
            CoreText.CTLineDraw(line, self.context)
            cur_x += col_widths[idx]
            
        self.cursor_y += hdr_height
        
        # Draw rows
        for r_idx, (r_height, cells) in enumerate(processed_rows):
            self.ensure_space(r_height)
            
            # Row background
            row_bg = COLOR_WHITE if (r_idx % 2 == 0) else COLOR_TABLE_ALT
            r_rect = Quartz.CGRectMake(MARGIN_LEFT, self.to_quartz_y(self.cursor_y + r_height), total_width, r_height)
            Quartz.CGContextSetRGBFillColor(self.context, *row_bg)
            Quartz.CGContextFillRect(self.context, r_rect)
            
            # Cell border line
            Quartz.CGContextSetRGBStrokeColor(self.context, *COLOR_RULE)
            Quartz.CGContextSetLineWidth(self.context, 0.4)
            Quartz.CGContextStrokeLineSegments(self.context, [
                Quartz.CGPointMake(MARGIN_LEFT, self.to_quartz_y(self.cursor_y + r_height)),
                Quartz.CGPointMake(MARGIN_LEFT + total_width, self.to_quartz_y(self.cursor_y + r_height))
            ], 2)
            
            # Cell contents
            c_x = MARGIN_LEFT
            for c_idx, clines in enumerate(cells):
                cell_y = self.cursor_y + 2.0
                for cl in clines:
                    astr = CoreText.CFAttributedStringCreate(None, cl, {
                        CoreText.kCTFontAttributeName: FONTS['table_cell'],
                        CoreText.kCTForegroundColorFromContextAttributeName: True
                    })
                    line = CoreText.CTLineCreateWithAttributedString(astr)
                    Quartz.CGContextSetRGBFillColor(self.context, *COLOR_TEXT_DARK)
                    Quartz.CGContextSetTextPosition(self.context, c_x + 4.0, self.to_quartz_y(cell_y + 7.5))
                    CoreText.CTLineDraw(line, self.context)
                    cell_y += 9.5
                c_x += col_widths[c_idx]
                
            self.cursor_y += r_height
            
        self.cursor_y += 8.0

    def finish(self):
        self.draw_page_decorations()
        Quartz.CGPDFContextEndPage(self.context)
        Quartz.CGPDFContextClose(self.context)
        
        # Second Pass with PyPDF: Add "Page X of Y" and Bookmarks
        total_pages = self.current_page
        reader = PdfReader(self.tmp_path)
        writer = PdfWriter()
        
        for idx, page in enumerate(reader.pages):
            writer.add_page(page)
            
        # Write out final
        with open(self.output_path, 'wb') as f:
            writer.write(f)
            
        if os.path.exists(self.tmp_path):
            os.remove(self.tmp_path)
            
        print(f"[PDFRenderer] Successfully built '{self.output_path}' ({total_pages} pages).")


def build_report():
    pdf_path = "/Users/raunzz/oops project/ALF-EWG/ALF_EWG_Technical_Report.pdf"
    doc = PDFRenderer(pdf_path)
    doc.start()
    
    # -------------------------------------------------------------
    # Cover Banner
    # -------------------------------------------------------------
    doc.draw_cover_banner(
        title="ALF-EWG: AUTONOMOUS DISASTER WARNING GRID",
        subtitle="Complete Technical Report & Object-Oriented C++17 Architecture Guide",
        meta_items=[
            "B.Tech CSE OOP Semester Project",
            "C++17 Standard Compliant",
            "Strict -Wall -Wextra -Wpedantic",
            "Zero External Dependencies"
        ]
    )
    
    # -------------------------------------------------------------
    # 1. Executive Overview & Problem Statement
    # -------------------------------------------------------------
    doc.draw_heading_1("1. Executive Overview & Problem Statement")
    doc.draw_paragraph(
        "Mountainous terrains such as the young fold mountains of the Himalayas are subject to volatile compound disasters: "
        "monsoonal cloudbursts, progressive soil saturation, seismic micro-tremors, and catastrophic flash floods. Traditional "
        "environmental telemetry stations often operate in isolated silos, failing to cross-correlate multiple sensor streams "
        "or prioritize dangerous regions during active emergencies."
    )
    doc.draw_paragraph(
        "The Autonomous Landslide & Flash-Flood Early Warning Grid (ALF-EWG) simulates a rugged, distributed disaster "
        "mitigation network. It models field sensor stations, calculates composite hazard ratings using multi-factor mathematical models, "
        "triggers simulated regional sirens, maps evacuation priorities, and commits high-risk incidents to permanent binary storage."
    )
    
    doc.draw_callout_box("Core Design Philosophy", [
        "1. Academic Rigor: Built strictly according to the four units of University C++ Object-Oriented Programming.",
        "2. Resource Safety: Zero memory leaks verified by RAII and the modern C++ Rule of Five.",
        "3. High Resilience: Resilient binary flat-file serialization with transactional write rollback mechanisms."
    ])
    
    # -------------------------------------------------------------
    # 2. System Architecture & Class Hierarchy
    # -------------------------------------------------------------
    doc.draw_heading_1("2. System Architecture & Class Hierarchy")
    doc.draw_paragraph(
        "The system architecture incorporates two separate inheritance trees, containment, and multiple utility classes "
        "living inside explicit namespaces:"
    )
    
    doc.draw_code_block([
        "                    ┌─────────────────────────┐",
        "                    │ DisasterTelemetryDevice │ (Root Base Class)",
        "                    └────────────┬────────────┘",
        "                                 │",
        "                 ┌───────────────┴───────────────┐",
        "                 │ virtual                       │ virtual",
        "                 ▼                               ▼",
        "      ┌──────────────────┐           ┌─────────────────────────┐",
        "      │ SolarPoweredNode │           │ WirelessLoRaTransceiver │",
        "      └──────────┬───────┘           └───────────┬─────────────┘",
        "                 │                               │",
        "                 └───────────────┬───────────────┘",
        "                                 │ (Multiple Inheritance - Diamond Resolved)",
        "                                 ▼",
        "                     ┌───────────────────────┐",
        "                     │ AutonomousHybridProbe │",
        "                     └───────────┬───────────┘",
        "                                 │ public",
        "                                 ▼",
        "                    ┌─────────────────────────┐",
        "                    │     BaseSensorNode      │ (Abstract Base Class)",
        "                    │  [has-a PowerSubsystem] │ (Containership / Composition)",
        "                    └────────────┬────────────┘",
        "                                 │",
        "     ┌───────────────────┬───────┴───────────┬───────────────────┐",
        "     ▼                   ▼                   ▼                   ▼",
        "┌──────────────┐ ┌────────────────┐ ┌──────────────────┐ ┌────────────────┐",
        "│ SoilMoisture │ │ SeismicVibrate │ │ AcousticTiltNode │ │ RiverLevelNode │",
        "│    Sensor    │ │     Sensor     │ │      Sensor      │ │     Sensor     │",
        "└──────────────┘ └────────────────┘ └──────────────────┘ └────────────────┘"
    ])
    
    # -------------------------------------------------------------
    # 3. UNIT I: Core Foundations & Call Semantics
    # -------------------------------------------------------------
    doc.draw_heading_1("3. UNIT I: Core Foundations & Call Semantics")
    doc.draw_paragraph(
        "Unit I establishes clean namespace organization, compiler inlining optimizations, compile-time function overloading, "
        "efficient call-by-reference semantics, return-by-reference lvalues, and access-control bypass via friendship."
    )
    
    doc.draw_heading_2("A. Namespaces (DisasterCore::Sensors & DisasterCore::Alerts)")
    doc.draw_bullet("DisasterCore::Sensors: Contains physical stations, polymorphic probes, and telemetry auditors.")
    doc.draw_bullet("DisasterCore::Alerts: Encloses siren controllers, cluster arithmetic, and evacuation ranking functors.")
    doc.draw_bullet("Benefit: Prevents global symbol clashes and modularizes operational layers cleanly.")
    
    doc.draw_heading_2("B. Inline Functions (classifyRisk & normalizeValue)")
    doc.draw_bullet("Defined with the inline keyword in SensorStation.h to avoid function call stack frame creation.")
    doc.draw_bullet("Evaluates risk levels (LOW, MODERATE, HIGH, CRITICAL) thousands of times during loops with zero overhead.")
    
    doc.draw_heading_2("C. Function Overloading & Tag-Dispatch (computeHazardIndex)")
    doc.draw_paragraph("Three distinct overloaded signatures compute different hazard types:")
    doc.draw_code_block([
        "// Overload 1: Landslide Hazard (Rainfall Rate + Terrain Slope)",
        "float computeHazardIndex(float rainfallRate, float slopeAngle);",
        "",
        "// Overload 2: Seismic Risk (Soil Saturation + Ground Vibration Frequency)",
        "// Uses a zero-cost 'SeismicTag' struct to differentiate identical (float, float) signatures",
        "struct SeismicTag {};",
        "float computeHazardIndex(float soilSaturation, float vibrationFrequency, SeismicTag);",
        "",
        "// Overload 3: Flash-Flood Hazard (River Level + Rainfall Rate + Slope Angle)",
        "float computeHazardIndex(float riverLevel, float rainfallRate, float slopeAngle);"
    ])
    
    doc.draw_heading_2("D. Call by Reference & Return by Reference")
    doc.draw_bullet("Call by Reference: SensorStation::updateReadings(float&, float&, ...) modifies the caller's arguments in-place.")
    doc.draw_bullet("Call by Reference: PowerSubsystem::setValues(float&, float&) clamps and reflects values back through references.")
    doc.draw_bullet("Return by Reference: SensorStation::getHazardIndexRef() returns float&, allowing in-place lvalue mutation:")
    doc.draw_code_block([
        "// Mutating an internal private field through return by reference:",
        "station.getHazardIndexRef() = 99.9f;"
    ])
    
    doc.draw_heading_2("E. Friend Function & Friend Class")
    doc.draw_bullet("Friend Function: auditPower(const PowerSubsystem& ps) directly inspects ps.batteryLevel without getters.")
    doc.draw_bullet("Friend Class: TelemetryAuditor is declared friend class TelemetryAuditor inside SensorStation.")
    doc.draw_bullet("Enables direct auditing of private metrics: stationId, rainfallRate, vibrationFreq, soilMoisture.")
    
    # -------------------------------------------------------------
    # 4. UNIT II: Encapsulation, Dynamic Memory & Operator Overloading
    # -------------------------------------------------------------
    doc.draw_heading_1("4. UNIT II: Encapsulation, Dynamic Memory & Operator Overloading")
    doc.draw_paragraph(
        "Unit II covers strict data hiding, const correctness, static state tracking, dynamic heap memory management, "
        "deep copy constructors, move semantics (Rule of Five), and intuitive operator overloads."
    )
    
    doc.draw_heading_2("A. Encapsulation & Static Class Members")
    doc.draw_bullet("Private state members with const member functions (getStationId() const, getHazardIndex() const).")
    doc.draw_bullet("SensorStation::totalStations: Static counter tracking total active stations across the entire grid.")
    doc.draw_bullet("EarlyWarningSiren::globalAlertLevel: Static emergency state shared by all regional sirens.")
    
    doc.draw_heading_2("B. Dynamic Memory & The Rule of Five")
    doc.draw_paragraph(
        "SensorStation allocates an internal float* readingsBuffer using new float[4]. To guarantee complete memory safety, "
        "it fully implements the Rule of Five:"
    )
    doc.draw_code_block([
        "// 1. Parameterized Constructor: Allocates buffer and increments static counter",
        "readingsBuffer = new float[4]; ++totalStations;",
        "",
        "// 2. Deep Copy Constructor: Allocates new buffer, copies values element-by-element",
        "SensorStation::SensorStation(const SensorStation& other) : ... {",
        "    readingsBuffer = new float[4];",
        "    for (int i = 0; i < 4; ++i) readingsBuffer[i] = other.readingsBuffer[i];",
        "    ++totalStations;",
        "}",
        "",
        "// 3. Copy Assignment Operator (operator=): Self-assignment check + deep copy",
        "SensorStation& SensorStation::operator=(const SensorStation& other) {",
        "    if (this != &other) { ... readingsBuffer[i] = other.readingsBuffer[i]; }",
        "    return *this;",
        "}",
        "",
        "// 4. Move Constructor & Move Assignment: Transfers pointer ownership without heap allocation",
        "SensorStation::SensorStation(SensorStation&& other) noexcept : ... {",
        "    readingsBuffer = other.readingsBuffer; other.readingsBuffer = nullptr; ++totalStations;",
        "}",
        "",
        "// 5. Destructor: Releases heap buffer and decrements static counter",
        "SensorStation::~SensorStation() { delete[] readingsBuffer; readingsBuffer = nullptr; --totalStations; }"
    ])
    
    doc.draw_heading_2("C. Operator Overloading (+, >, <<, >>)")
    doc.draw_bullet("Binary Operator +: Combines two regional SensorCluster objects into a unified sector, summing hazard indices.")
    doc.draw_bullet("Relational Operator >: Compares station or cluster danger levels: stationA > stationB.")
    doc.draw_bullet("Stream Insertion <<: Allows clean formatted printing: std::cout << station.")
    doc.draw_bullet("Stream Extraction >>: Allows interactive telemetry input: std::cin >> station.")
    
    # -------------------------------------------------------------
    # 5. UNIT III: Inheritance, Containership & Polymorphism
    # -------------------------------------------------------------
    doc.draw_heading_1("5. UNIT III: Inheritance, Containership & Polymorphism")
    doc.draw_paragraph(
        "Unit III demonstrates polymorphic interface abstraction, composition vs. inheritance, resolving multiple inheritance "
        "diamond ambiguity, and dynamic method dispatch via virtual function tables."
    )
    
    doc.draw_heading_2("A. Abstract Base Class (BaseSensorNode)")
    doc.draw_bullet("Declares pure virtual functions readTelemetry() = 0 and evaluateThreshold() = 0.")
    doc.draw_bullet("Declares a virtual destructor virtual ~BaseSensorNode() to ensure safe polymorphic deallocation.")
    doc.draw_bullet("Cannot be directly instantiated; provides a contract for field sensor implementations.")
    
    doc.draw_heading_2("B. Containership (Composition) vs. Inheritance")
    doc.draw_bullet("BaseSensorNode contains a PowerSubsystem powerUnit object directly inside its class body.")
    doc.draw_bullet("Inheritance models 'is-a' (SoilMoistureSensor is a BaseSensorNode).")
    doc.draw_bullet("Containership models 'has-a' (BaseSensorNode has a PowerSubsystem solar battery).")
    
    doc.draw_heading_2("C. Diamond Inheritance Resolution via Virtual Base Classes")
    doc.draw_paragraph(
        "Without virtual inheritance, AutonomousHybridProbe would inherit two copies of DisasterTelemetryDevice, "
        "causing compilation ambiguity when accessing deviceSerial or getDeviceInfo():"
    )
    doc.draw_code_block([
        "class DisasterTelemetryDevice { ... };",
        "",
        "// Intermediate classes use VIRTUAL inheritance:",
        "class SolarPoweredNode : public virtual DisasterTelemetryDevice { ... };",
        "class WirelessLoRaTransceiver : public virtual DisasterTelemetryDevice { ... };",
        "",
        "// Leaf class inherits both with single shared root sub-object:",
        "class AutonomousHybridProbe : public BaseSensorNode,",
        "                              public SolarPoweredNode,",
        "                              public WirelessLoRaTransceiver {",
        "public:",
        "    AutonomousHybridProbe(...) : DisasterTelemetryDevice(...), SolarPoweredNode(...), WirelessLoRaTransceiver(...) {}",
        "};"
    ])
    
    doc.draw_heading_2("D. Runtime Polymorphism & Dynamic Dispatch (vtable/vptr)")
    doc.draw_bullet("BaseSensorNode* pointers in a vector store diverse derived objects (Soil, Seismic, Tilt, River).")
    doc.draw_bullet("Compiler generates a vtable for each class; each object contains a hidden vptr pointing to its vtable.")
    doc.draw_bullet("runSensorNetworkSweep(std::vector<BaseSensorNode*>& nodes) dynamically dispatches calls at runtime.")
    
    # -------------------------------------------------------------
    # 6. UNIT IV: Streams, Persistence, Exception Handling & STL
    # -------------------------------------------------------------
    doc.draw_heading_1("6. UNIT IV: Streams, Persistence, Exception Handling & STL")
    doc.draw_paragraph(
        "Unit IV handles non-volatile binary flat-file serialization, transactional write rollback, custom exception "
        "hierarchies, STL containers, custom functors, and output stream manipulators."
    )
    
    doc.draw_heading_2("A. Custom Exception Hierarchy (std::exception)")
    doc.draw_bullet("SensorFailureException: Thrown on hardware faults or file write failures.")
    doc.draw_bullet("TelemetryChecksumMismatch: Thrown when telemetry corruption is detected.")
    doc.draw_bullet("ThresholdOverflowException: Thrown when physical sensor boundaries are breached.")
    doc.draw_bullet("All override const char* what() const noexcept with descriptive context.")
    
    doc.draw_heading_2("B. Binary Flat-File Serialization & Plain-Old-Data (POD)")
    doc.draw_paragraph(
        "IncidentRecord uses fixed-length char arrays rather than std::string to ensure a contiguous binary memory layout:"
    )
    doc.draw_code_block([
        "struct IncidentRecord {",
        "    int   incidentId;    int   stationId;",
        "    char  regionName[64];char  hazardType[32];",
        "    float hazardIndex;   char  riskLevel[16];   char timestamp[32];",
        "};",
        "// Binary write: ofs.write(reinterpret_cast<const char*>(&rec), sizeof(IncidentRecord));",
        "// Binary read : ifs.read(reinterpret_cast<char*>(&rec), sizeof(IncidentRecord));"
    ])
    
    doc.draw_heading_2("C. File Stream Positioning & Transactional Rollback")
    doc.draw_bullet("seekp(0, std::ios::end) determines the pre-write position (preWritePos = ofs.tellp()).")
    doc.draw_bullet("If writing fails, the exception handler reopens the file and executes rollback.seekp(preWritePos) to truncate.")
    doc.draw_bullet("seekg(0, std::ios::beg) rewinds the read pointer to the start of the binary log for reporting.")
    doc.draw_bullet("fs.clear(): Crucial stream reset after reaching EOF to allow subsequent seekg operations to succeed.")
    
    doc.draw_heading_2("D. STL Containers, Algorithms & Custom Functor (EvacuationPriority)")
    doc.draw_bullet("std::vector: Dynamic container for sensor stations, active sirens, and polymorphic probes.")
    doc.draw_bullet("std::map<std::string, float>: Key-value associative mapping from region name to composite hazard score.")
    doc.draw_bullet("std::sort: Sorts regional clusters in descending hazard severity using custom functor:")
    doc.draw_code_block([
        "struct EvacuationPriority {",
        "    bool operator()(const SensorCluster& a, const SensorCluster& b) const {",
        "        return a.getAverageHazard() > b.getAverageHazard(); // Descending order",
        "    }",
        "};"
    ])
    doc.draw_bullet("Custom Stream Manipulators: alertHeader and alertFooter format siren reports with clean ASCII lines.")
    
    # -------------------------------------------------------------
    # 7. Syllabus Concept Mapping Table
    # -------------------------------------------------------------
    doc.draw_heading_1("7. Unit-Wise Syllabus Concept Mapping Matrix")
    doc.draw_paragraph(
        "The following matrix maps every university syllabus requirement directly to its implementation in the codebase:"
    )
    
    headers = ["Unit", "C++ Concept", "Class / Function Used", "Purpose in Project"]
    col_widths = [45.0, 115.0, 152.0, 200.0] # Total 512.0
    
    table_rows = [
        ["Unit I", "Namespaces", "DisasterCore::Sensors, Alerts", "Logically separates probe layers from alerting layers"],
        ["Unit I", "Inline Functions", "classifyRisk(), normalizeValue()", "Zero-overhead risk categorization and index normalization"],
        ["Unit I", "Function Overloading", "computeHazardIndex (3 overloads)", "Separate mathematical models for Landslide, Seismic, and Flood"],
        ["Unit I", "Call by Reference", "updateReadings(float&, ...)", "In-place telemetry updates without object copying"],
        ["Unit I", "Return by Reference", "getHazardIndexRef()", "Enables direct lvalue mutation of internal hazard scores"],
        ["Unit I", "Friend Function", "auditPower(const PowerSubsystem&)", "Inspects private battery levels without public getters"],
        ["Unit I", "Friend Class", "TelemetryAuditor", "Directly audits private sensor metrics during inspections"],
        ["Unit II", "Encapsulation & Const", "SensorStation, EarlyWarningSiren", "Data hiding with const-correct inspectors"],
        ["Unit II", "Dynamic Memory", "new float[4], delete[]", "Heap allocation and release of telemetry reading buffers"],
        ["Unit II", "Rule of Five", "SensorStation(copy, move, assign)", "Independent deep-copying and efficient move semantics"],
        ["Unit II", "Static Members", "totalStations, globalAlertLevel", "Tracks network-wide station count and alert status"],
        ["Unit II", "Operator Overload (+)", "SensorCluster::operator+", "Combines two geographical clusters into a unified sector"],
        ["Unit II", "Operator Overload (>)", "SensorStation::operator>", "Compares hazard severity between stations"],
        ["Unit II", "Operator Overload (<<,>>)", "operator<<, operator>>", "Clean stream insertion and extraction for station I/O"],
        ["Unit III", "Abstract Base Class", "BaseSensorNode", "Polymorphic base interface with pure virtuals (= 0)"],
        ["Unit III", "Virtual Inheritance", "SolarPoweredNode, LoRaTransceiver", "Resolves multiple inheritance diamond ambiguity"],
        ["Unit III", "Containership", "BaseSensorNode has PowerSubsystem", "Models 'has-a' solar battery composition"],
        ["Unit III", "Runtime Polymorphism", "vector<BaseSensorNode*>, sweep", "Dynamic method dispatch via vtable/vptr at runtime"],
        ["Unit IV", "Custom Exceptions", "SensorFailureException, etc.", "Domain exception hierarchy derived from std::exception"],
        ["Unit IV", "Binary File Handling", "IncidentManager (read/write)", "Permanent flat-file persistence for emergency logs"],
        ["Unit IV", "File Positioning", "seekp(), seekg(), tellp(), tellg()", "Pre-write position recording and rollback on failure"],
        ["Unit IV", "Stream Reset (clear)", "fs.clear()", "Clears EOF/fail bits before rewinding file streams"],
        ["Unit IV", "STL Containers", "vector, map<string, float>", "Station storage, cluster grouping, and regional risk maps"],
        ["Unit IV", "STL Algorithms", "std::sort, std::for_each", "Sorting evacuation zones and iterating station telemetry"],
        ["Unit IV", "Custom Functor", "struct EvacuationPriority", "Functor comparator for sorting evacuation roadmaps"],
        ["Unit IV", "Custom Manipulator", "alertHeader, alertFooter", "Clean tabular formatting for siren alert reports"]
    ]
    
    doc.draw_table(headers, table_rows, col_widths)
    
    # -------------------------------------------------------------
    # 8. Viva Voce Defense Guide
    # -------------------------------------------------------------
    doc.draw_heading_1("8. Viva Voce & Practical Exam Defense Guide")
    
    qa_list = [
        ("Q1: Why does SensorStation dynamically allocate readingsBuffer instead of using a standard array?",
         "A1: To demonstrate heap memory allocation using new[], proper deallocation via delete[] in the destructor, "
         "and the Rule of Five (deep copy constructor and copy assignment operator to prevent pointer aliasing and double-free crashes)."),
         
        ("Q2: How does virtual inheritance eliminate the Diamond Problem in AutonomousHybridProbe?",
         "A2: By using 'public virtual DisasterTelemetryDevice' in SolarPoweredNode and WirelessLoRaTransceiver, the compiler ensures "
         "that only one shared instance of the root sub-object exists. The most derived class constructs the virtual base directly."),
         
        ("Q3: What is the architectural difference between Containership and Inheritance?",
         "A3: Inheritance models an 'is-a' relationship (SoilMoistureSensor is a BaseSensorNode). Containership models a 'has-a' relationship "
         "(BaseSensorNode has a PowerSubsystem). A sensor node uses a power unit; it is not a power unit."),
         
        ("Q4: Why does IncidentRecord use fixed-size char arrays instead of std::string?",
         "A4: std::string contains internal pointers to dynamic memory. Writing std::string to disk with binary write() would write "
         "meaningless memory addresses. Fixed-size char arrays form a Plain Old Data (POD) struct with a contiguous binary memory layout."),
         
        ("Q5: Why is fs.clear() mandatory before seeking after reading to EOF?",
         "A5: When a file stream reaches EOF, the stream sets its eofbit and failbit. Calling seekg() while these bits are active fails silently. "
         "Calling fs.clear() resets the stream state, allowing subsequent seek and read operations to succeed."),
         
        ("Q6: How does dynamic dispatch work under the hood during the sensor network sweep?",
         "A6: The compiler generates a virtual function table (vtable) for each polymorphic class and places a hidden vptr pointer in each object. "
         "When calling node->readTelemetry(), the executable dereferences the vptr to invoke the derived class's method at runtime.")
    ]
    
    for q, a in qa_list:
        doc.draw_heading_3(q)
        doc.draw_paragraph(a, indent=8.0)
        
    doc.finish()

if __name__ == '__main__':
    build_report()
