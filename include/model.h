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
    int32_t *vert_x;
    int32_t *vert_y;
    int32_t *vert_z;
    int32_t *vert_shade;
    int32_t *vert_ambient;

    size_t max_face_cnt;
    size_t face_cnt;
    int32_t *face_vert_cnt;
    int32_t *face_verts;
    int32_t *texture_front;
    int32_t *texture_back;
    int32_t *light_type;
    int32_t *norm_scale;
    int32_t *norm_mag;

    int32_t *project_vert_x;
    int32_t *project_vert_y;
    int32_t *project_vert_z;
    int32_t *project_plane_x;
    int32_t *project_plane_y;

    int8_t *is_local;
    int32_t *face_pick_tag;

    int32_t *trans_vert_x;
    int32_t *trans_vert_y;
    int32_t *trans_vert_z;

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
    int32_t max_dim;

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

#endif // MODEL_H
