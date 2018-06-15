#ifndef MODEL_H
#define MODEL_H

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <jag_arc.h>

#define MODEL_USE_GOURAD 12345678

struct model {
    size_t max_vert_cnt;
    size_t vert_cnt;
    int16_t *vert_x;
    int16_t *vert_y;
    int16_t *vert_z;
    int32_t *vert_shade;
    int32_t *vert_ambient;

    size_t max_face_cnt;
    size_t face_cnt;
    uint8_t *face_vert_cnt;
    int16_t **face_verts;
    int32_t *texture_front;
    int32_t *texture_back;
    uint16_t *light_type;
    int32_t *norm_scale;
    int32_t *norm_mag;

    int32_t *project_vert_x;
    int32_t *project_vert_y;
    int32_t *project_vert_z;
    int32_t *project_plane_x;
    int32_t *project_plane_y;

    int8_t *is_local;
    int32_t *face_pick_tag;

    int16_t *trans_vert_x;
    int16_t *trans_vert_y;
    int16_t *trans_vert_z;

    int32_t *face_norm_x;
    int32_t *face_norm_y;
    int32_t *face_norm_z;

    int32_t *face_min_x;
    int32_t *face_max_x;
    int32_t *face_min_y;
    int32_t *face_max_y;
    int32_t *face_min_z;
    int32_t *face_max_z;

    bool visible;
    bool unrendered;
    bool unpickable;
    bool autocommit;
    bool no_shading;
    bool no_bounds;
    bool trans;
    bool trans_texture;

    int32_t max_dim;
    int32_t min_x;
    int32_t min_y;
    int32_t min_z;
    int32_t max_x;
    int32_t max_y;
    int32_t max_z;

    int32_t light_x;
    int32_t light_y;
    int32_t light_z;
    int32_t light_mag;
    int32_t light_falloff;
    int32_t light_ambient;

    int32_t base_x;
    int32_t base_y;
    int32_t base_z;

    int32_t yaw;
    int32_t pitch;
    int32_t roll;

    int32_t depth;

    int32_t scale_x;
    int32_t scale_y;
    int32_t scale_z;

    int32_t shear_xy;
    int32_t shear_zy;
    int32_t shear_xz;
    int32_t shear_yz;
    int32_t shear_zx;
    int32_t shear_yx;

    int32_t trans_state;
    int32_t trans_type;
};

void model_init(struct model *model, size_t vert_cnt, size_t face_cnt);

void model_decode(struct model *model, struct jag_arc_entry *entry);

void model_deinit(struct model *model);

void model_project_prepare(struct model *model);

void model_clear(struct model *model);

void model_reduce(struct model *model, size_t face_cnt, size_t vert_cnt);

void model_join(struct model *model, struct model *parts, size_t size);

size_t model_vert_add(struct model *model, int16_t x, int16_t y, int16_t z);

size_t model_vert_get(struct model *model, int16_t x, int16_t y, int16_t z);

size_t model_face_add(struct model *model, size_t vert_cnt, int16_t *verts,
                      int32_t front, int32_t back);

void model_scale(struct model *model, int32_t x, int32_t y, int32_t z);

void model_shear(struct model *model, int32_t xy, int32_t zy, int32_t xz,
                 int32_t yz, int32_t zx, int32_t yx);

void model_rotate(struct model *model, uint8_t yaw, uint8_t pitch,
                  uint8_t roll);

void model_orient(struct model *model, uint8_t yaw, uint8_t pitch,
                  uint8_t roll);

void model_translate(struct model *model, int32_t x, int32_t y, int32_t z);

void model_position(struct model *model, int32_t x, int32_t y, int32_t z);

void model_commit(struct model *model);

void model_trans_apply(struct model *model);

void model_calc_norms(struct model *model);

#endif // MODEL_H
