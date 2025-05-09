﻿/**
 * File:   lcd.c
 * Author: Li XianJing <xianjimli@hotmail.com>
 * Brief:  vgcanvasi implemented lcd_t
 *
 * Copyright (c) 2018 - 2020  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-04-11 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "base/lcd.h"
#include "tkc/mem.h"
#include "tkc/utf8.h"
#include "tkc/str.h"
#include "base/vgcanvas.h"
#include "base/system_info.h"

typedef struct _lcd_vgcanvas_t {
  lcd_t base;

  vgcanvas_t* canvas;
  str_t temp_text;
} lcd_vgcanvas_t;

#define LCD(lcd) ((lcd_vgcanvas_t*)lcd)

static ret_t lcd_vgcanvas_resize(lcd_t* lcd, wh_t w, wh_t h, uint32_t line_length) {
  lcd->w = w;
  lcd->h = h;
  LCD(lcd)->canvas->w = w;
  LCD(lcd)->canvas->h = h;
  LCD(lcd)->canvas->dirty_rect.w = w;
  LCD(lcd)->canvas->dirty_rect.h = h;
  (void)line_length;

  return RET_OK;
}

static ret_t lcd_vgcanvas_set_orientation(lcd_t* lcd, lcd_orientation_t old_orientation, lcd_orientation_t new_orientation) {
  if (tk_is_swap_size_by_orientation(old_orientation, new_orientation)) {
    return lcd_vgcanvas_resize(lcd, lcd->h, lcd->w, 0);
  }
  return RET_OK;
}

static ret_t lcd_vgcanvas_set_font_name(lcd_t* lcd, const char* name) {
  return vgcanvas_set_font(LCD(lcd)->canvas, name);
}

static ret_t lcd_vgcanvas_set_font_size(lcd_t* lcd, uint32_t size) {
  return vgcanvas_set_font_size(LCD(lcd)->canvas, size);
}

static ret_t lcd_vgcanvas_begin_frame(lcd_t* lcd, const dirty_rects_t* dirty_rects) {
  return vgcanvas_begin_frame(LCD(lcd)->canvas, dirty_rects);
}

static ret_t lcd_vgcanvas_set_clip_rect(lcd_t* lcd, const rect_t* r) {
  return vgcanvas_clip_rect(LCD(lcd)->canvas, r->x, r->y, r->w, r->h);
}

static bool_t lcd_vgcanvas_is_rect_in_clip_rect(lcd_t* lcd, xy_t left, xy_t top, xy_t right, xy_t bottom) {
  vgcanvas_t* canvas = LCD(lcd)->canvas;
  return vgcanvas_is_rectf_int_clip_rect(canvas, (float_t)left, (float_t)top, (float_t)right, (float_t)bottom);
}

static ret_t lcd_vgcanvas_get_clip_rect(lcd_t* lcd, rect_t* r) {
  vgcanvas_t* canvas = LCD(lcd)->canvas;
  *r = rect_from_rectf(vgcanvas_get_clip_rect(canvas));
  return RET_OK;
}

static ret_t lcd_vgcanvas_set_global_alpha(lcd_t* lcd, uint8_t alpha) {
  return vgcanvas_set_global_alpha(LCD(lcd)->canvas, (float_t)alpha / 255.0);
}

static ret_t lcd_vgcanvas_draw_vline_with_color(lcd_t* lcd, xy_t x, xy_t y, wh_t h, color_t stroke_color) {
  vgcanvas_t* canvas = LCD(lcd)->canvas;

  vgcanvas_save(canvas);
  vgcanvas_begin_path(canvas);
  vgcanvas_set_antialias(canvas, FALSE);
  vgcanvas_translate(canvas, 0.5f, 0.0f);
  vgcanvas_move_to(canvas, x, y);
  vgcanvas_line_to(canvas, x, y + h);
  vgcanvas_set_line_width(canvas, 1);
  vgcanvas_set_stroke_color(canvas, stroke_color);
  vgcanvas_stroke(canvas);
  vgcanvas_set_antialias(canvas, TRUE);
  vgcanvas_restore(canvas);
  return RET_OK;
}

static ret_t lcd_vgcanvas_draw_vline(lcd_t* lcd, xy_t x, xy_t y, wh_t h) {
  return lcd_vgcanvas_draw_vline_with_color(lcd, x, y, h, lcd->stroke_color);
}

static ret_t lcd_vgcanvas_draw_hline_with_color(lcd_t* lcd, xy_t x, xy_t y, wh_t w, color_t stroke_color) {
  vgcanvas_t* canvas = LCD(lcd)->canvas;

  vgcanvas_save(canvas);
  vgcanvas_begin_path(canvas);
  vgcanvas_set_antialias(canvas, FALSE);
  vgcanvas_translate(canvas, 0.0f, 0.5f);
  vgcanvas_move_to(canvas, x, y);
  vgcanvas_line_to(canvas, x + w, y);
  vgcanvas_set_line_width(canvas, 1);
  vgcanvas_set_stroke_color(canvas, stroke_color);
  vgcanvas_stroke(canvas);
  vgcanvas_set_antialias(canvas, TRUE);
  vgcanvas_restore(canvas);
  return RET_OK;
}

static ret_t lcd_vgcanvas_draw_hline(lcd_t* lcd, xy_t x, xy_t y, wh_t w) {
  return lcd_vgcanvas_draw_hline_with_color(lcd, x, y, w, lcd->stroke_color);
}

static ret_t lcd_vgcanvas_fill_rect(lcd_t* lcd, xy_t x, xy_t y, wh_t w, wh_t h) {
  vgcanvas_t* canvas = LCD(lcd)->canvas;

  if (w > 1 && h > 1) {
    vgcanvas_save(canvas);
    vgcanvas_begin_path(canvas);
    vgcanvas_set_antialias(canvas, FALSE);
    vgcanvas_rect(canvas, x, y, w, h);
    vgcanvas_set_fill_color(canvas, lcd->fill_color);
    vgcanvas_fill(canvas);
    vgcanvas_set_antialias(canvas, TRUE);
    vgcanvas_restore(canvas);
  } else if (w <= 1) {
    lcd_vgcanvas_draw_vline_with_color(lcd, x, y, h, lcd->fill_color);
  } else if (h <= 1) {
    lcd_vgcanvas_draw_hline_with_color(lcd, x, y, w, lcd->fill_color);
  }

  return RET_OK;
}

static ret_t lcd_vgcanvas_clear_rect(lcd_t* lcd, xy_t x, xy_t y, wh_t w, wh_t h) {
  vgcanvas_t* canvas = LCD(lcd)->canvas;

  vgcanvas_save(canvas);
  vgcanvas_clear_rect(canvas, x, y, w, h, lcd->fill_color);
  vgcanvas_restore(canvas);
  return RET_OK;
}

static ret_t lcd_vgcanvas_stroke_rect(lcd_t* lcd, xy_t x, xy_t y, wh_t w, wh_t h) {
  float_t offset_1 = 1.0f / lcd->ratio;
  float_t offset_5 = 0.5f / lcd->ratio;
  vgcanvas_t* canvas = LCD(lcd)->canvas;

  vgcanvas_begin_path(canvas);
  vgcanvas_rect(canvas, x + offset_5, y + offset_5, w - offset_1, h - offset_1);
  vgcanvas_set_line_width(canvas, 1);
  vgcanvas_set_stroke_color(canvas, lcd->stroke_color);
  vgcanvas_stroke(canvas);

  return RET_OK;
}

static ret_t lcd_vgcanvas_draw_points(lcd_t* lcd, point_t* points, uint32_t nr) {
  uint32_t i = 0;
  float_t offset = 0.5f * lcd->ratio;
  vgcanvas_t* canvas = LCD(lcd)->canvas;
  vgcanvas_set_stroke_color(canvas, lcd->stroke_color);
  vgcanvas_set_line_width(canvas, 1);
  for (i = 0; i < nr; i++) {
    float x = points[i].x;
    float y = points[i].y;

    x += offset;

    vgcanvas_begin_path(canvas);
    vgcanvas_move_to(canvas, x, y + offset);
    vgcanvas_line_to(canvas, x, y);
    vgcanvas_stroke(canvas);
  }

  return RET_OK;
}

static ret_t lcd_vgcanvas_draw_image(lcd_t* lcd, bitmap_t* img, const rectf_t* src, const rectf_t* dst) {
  float_t sx = src->x;
  float_t sy = src->y;
  float_t sw = src->w;
  float_t sh = src->h;
  float_t dx = dst->x;
  float_t dy = dst->y;
  float_t dw = dst->w;
  float_t dh = dst->h;
  ret_t ret = RET_OK;
  vgcanvas_t* canvas = LCD(lcd)->canvas;

  vgcanvas_save(canvas);
  vgcanvas_set_antialias(canvas, FALSE);
  ret = vgcanvas_draw_image(canvas, img, sx, sy, sw, sh, dx, dy, dw, dh);
  vgcanvas_set_antialias(canvas, TRUE);
  vgcanvas_restore(canvas);

  return ret;
}

static ret_t lcd_vgcanvas_draw_image_repeat(lcd_t* lcd, bitmap_t* img, const rect_t* src, const rect_t* dst, wh_t dst_w, wh_t dst_h) {
  float_t sx = src->x;
  float_t sy = src->y;
  float_t sw = src->w;
  float_t sh = src->h;
  float_t dx = dst->x;
  float_t dy = dst->y;
  float_t dw = dst->w;
  float_t dh = dst->h;
  ret_t ret = RET_NOT_IMPL;
  vgcanvas_t* canvas = LCD(lcd)->canvas;
  if (canvas != NULL && canvas->vt->draw_image_repeat != NULL) {
    vgcanvas_save(canvas);
    vgcanvas_set_antialias(canvas, FALSE);
    ret = vgcanvas_draw_image_repeat(canvas, img, sx, sy, sw, sh, dx, dy, dw, dh, (float_t)dst_w, (float_t)dst_h);
    vgcanvas_set_antialias(canvas, TRUE);
    vgcanvas_restore(canvas);

  }
  return ret;
}

static ret_t lcd_vgcanvas_draw_image_matrix(lcd_t* lcd, draw_image_info_t* info) {
  matrix_t* m = &(info->matrix);
  const rect_t* s = &(info->src);
  const rect_t* d = &(info->dst);
  vgcanvas_t* canvas = LCD(lcd)->canvas;
  vgcanvas_save(canvas);
  vgcanvas_set_transform(canvas, m->a0, m->a1, m->a2, m->a3, m->a4, m->a5);
  vgcanvas_draw_image(canvas, info->img, s->x, s->y, s->w, s->h, d->x, d->y, d->w, d->h);
  vgcanvas_restore(canvas);

  return RET_OK;
}

float_t lcd_vgcanvas_measure_text(lcd_t* lcd, const wchar_t* str, uint32_t nr) {
  str_t* text = &(LCD(lcd)->temp_text);
  vgcanvas_t* canvas = LCD(lcd)->canvas;
  return_value_if_fail(str_from_wstr_with_len(text, str, nr) == RET_OK, RET_OOM);

  vgcanvas_set_font(canvas, lcd->font_name);
  vgcanvas_set_font_size(canvas, lcd->font_size);

  return vgcanvas_measure_text(LCD(lcd)->canvas, text->str);
}

static ret_t lcd_vgcanvas_draw_text(lcd_t* lcd, const wchar_t* str, uint32_t nr, xy_t x, xy_t y) {
  str_t* text = &(LCD(lcd)->temp_text);
  vgcanvas_t* canvas = LCD(lcd)->canvas;
  return_value_if_fail(str_from_wstr_with_len(text, str, nr) == RET_OK, RET_OOM);

  vgcanvas_set_font(canvas, lcd->font_name);
  vgcanvas_set_font_size(canvas, lcd->font_size);
  vgcanvas_set_fill_color(canvas, lcd->text_color);
  vgcanvas_set_text_align(canvas, "left");
  vgcanvas_set_text_baseline(canvas, "top");

  return vgcanvas_fill_text(canvas, text->str, x, y, 0xffff);
}

wh_t lcd_vgcanvas_get_width(lcd_t* lcd) {
  vgcanvas_t* canvas = LCD(lcd)->canvas;

  return vgcanvas_get_width(canvas);
}

wh_t lcd_vgcanvas_get_height(lcd_t* lcd) {
  vgcanvas_t* canvas = LCD(lcd)->canvas;

  return vgcanvas_get_height(canvas);
}

static ret_t lcd_vgcanvas_end_frame(lcd_t* lcd) {
  return vgcanvas_end_frame(LCD(lcd)->canvas);
}

static ret_t lcd_vgcanvas_destroy(lcd_t* lcd) {
  vgcanvas_t* canvas = LCD(lcd)->canvas;
  str_reset(&(LCD(lcd)->temp_text));
  vgcanvas_destroy(canvas);
  memset(lcd, 0x00, sizeof(lcd_t));
  TKMEM_FREE(lcd);

  return RET_OK;
}

static vgcanvas_t* lcd_vgcanvas_get_vgcanvas(lcd_t* lcd) {
  vgcanvas_t* canvas = LCD(lcd)->canvas;
  vgcanvas_begin_path(canvas);

  return canvas;
}

static bitmap_format_t lcd_vgcanvas_get_desired_bitmap_format(lcd_t* lcd) {
  return BITMAP_FMT_RGBA8888;
}

static ret_t lcd_vgcanvas_get_text_metrics(lcd_t* lcd, float_t* ascender, 
                                      float_t* descender, float_t* lineh) {
  vgcanvas_t* canvas = LCD(lcd)->canvas;
  return vgcanvas_get_text_metrics(canvas, ascender, descender, lineh);
}

lcd_t* lcd_vgcanvas_init(wh_t w, wh_t h, vgcanvas_t* canvas) {
  lcd_vgcanvas_t* lcd = TKMEM_ZALLOC(lcd_vgcanvas_t);
  lcd_t* base = &(lcd->base);
  system_info_t* info = system_info();
  return_value_if_fail(canvas != NULL, NULL);

  base->begin_frame = lcd_vgcanvas_begin_frame;
  base->set_clip_rect = lcd_vgcanvas_set_clip_rect;
  base->get_clip_rect = lcd_vgcanvas_get_clip_rect;
  base->is_rect_in_clip_rect = lcd_vgcanvas_is_rect_in_clip_rect;
  base->draw_vline = lcd_vgcanvas_draw_vline;
  base->draw_hline = lcd_vgcanvas_draw_hline;
  base->fill_rect = lcd_vgcanvas_fill_rect;
  base->clear_rect = lcd_vgcanvas_clear_rect;
  base->stroke_rect = lcd_vgcanvas_stroke_rect;
  base->draw_image = lcd_vgcanvas_draw_image;
  base->draw_image_repeat = lcd_vgcanvas_draw_image_repeat;
  base->draw_image_matrix = lcd_vgcanvas_draw_image_matrix;
  base->draw_points = lcd_vgcanvas_draw_points;
  base->draw_text = lcd_vgcanvas_draw_text;
  base->measure_text = lcd_vgcanvas_measure_text;
  base->end_frame = lcd_vgcanvas_end_frame;
  base->get_vgcanvas = lcd_vgcanvas_get_vgcanvas;
  base->set_font_name = lcd_vgcanvas_set_font_name;
  base->set_font_size = lcd_vgcanvas_set_font_size;
  base->set_global_alpha = lcd_vgcanvas_set_global_alpha;
  base->get_desired_bitmap_format = lcd_vgcanvas_get_desired_bitmap_format;
  base->resize = lcd_vgcanvas_resize;
  base->get_width = lcd_vgcanvas_get_width;
  base->get_height = lcd_vgcanvas_get_height;
  base->destroy = lcd_vgcanvas_destroy;
  base->get_text_metrics = lcd_vgcanvas_get_text_metrics;
  base->set_orientation = lcd_vgcanvas_set_orientation;
  base->w = (wh_t)w;
  base->h = (wh_t)h;
  base->ratio = canvas->ratio;
  base->type = LCD_VGCANVAS;
  base->support_dirty_rect = FALSE;

  system_info_set_lcd_w(info, base->w);
  system_info_set_lcd_h(info, base->h);
  system_info_set_lcd_type(info, base->type);
  system_info_set_device_pixel_ratio(info, canvas->ratio);

  str_init(&(lcd->temp_text), 100);
  lcd->canvas = canvas;
  
  return base;
}
