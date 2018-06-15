#include <model.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <buffer.h>
#include <log.h>
#include <math_ops.h>
#include <sin_table.h>

static void allocate(struct model *model, size_t vert_cnt, size_t face_cnt);

static void determine_trans(struct model *model);

static void rotate_apply(struct model *model);

static void scale_apply(struct model *model);

static void shear_apply(struct model *model);

static void translate_apply(struct model *model);

static void calc_bounds(struct model *model);

void model_init(struct model *model, size_t vert_cnt, size_t face_cnt) {
    memset(model, 0, sizeof(struct model));

    model->visible = true;
    model->max_dim = MODEL_USE_GOURAD;
    model->trans_state = 0;

    allocate(model, vert_cnt, face_cnt);
}

void model_decode(struct model *model, struct jag_arc_entry *entry) {
    memset(model, 0, sizeof(struct model));

    model->visible = true;
    model->max_dim = MODEL_USE_GOURAD;
    model->trans_state = 1;

    struct buffer buffer;
    buffer_wrap(&buffer, entry->data, entry->size);

    size_t vert_cnt = 0, face_cnt = 0;

    buffer_get2(&buffer, (uint16_t *)&vert_cnt);
    buffer_get2(&buffer, (uint16_t *)&face_cnt);

    allocate(model, vert_cnt, face_cnt);

    model->vert_cnt = vert_cnt;
    model->face_cnt = face_cnt;

    for (size_t i = 0; i < vert_cnt; i++) {
        buffer_get2(&buffer, (uint16_t *)&model->vert_x[i]);
    }

    for (size_t i = 0; i < vert_cnt; i++) {
        buffer_get2(&buffer, (uint16_t *)&model->vert_y[i]);
    }

    for (size_t i = 0; i < vert_cnt; i++) {
        buffer_get2(&buffer, (uint16_t *)&model->vert_z[i]);
    }

    for (size_t i = 0; i < face_cnt; i++) {
        buffer_get(&buffer, &model->face_vert_cnt[i]);
    }

    for (size_t i = 0; i < face_cnt; i++) {
        buffer_get2(&buffer, (uint16_t *)&model->texture_front[i]);

        if (model->texture_front[i] == 0x7FFF) {
            model->texture_front[i] = MODEL_USE_GOURAD;
        }
    }

    for (size_t i = 0; i < face_cnt; i++) {
        buffer_get2(&buffer, (uint16_t *)&model->texture_back[i]);

        if (model->texture_back[i] == 0x7FFF) {
            model->texture_back[i] = MODEL_USE_GOURAD;
        }
    }

    for (size_t i = 0; i < face_cnt; i++) {
        buffer_get(&buffer, (uint8_t *)&model->light_type[i]);

        if (model->light_type[i] != 0) {
            model->light_type[i] = (uint16_t)MODEL_USE_GOURAD;
        }
    }

    for (size_t i = 0; i < face_cnt; i++) {
        model->face_verts[i] =
            malloc(sizeof(int16_t) * model->face_vert_cnt[i]);

        for (size_t j = 0; j < model->face_vert_cnt[i]; j++) {
            if (vert_cnt < 256) {
                buffer_get(&buffer, (uint8_t *)&model->face_verts[i][j]);
            } else {
                buffer_get2(&buffer, (uint16_t *)&model->face_verts[i][j]);
            }
        }
    }
}

void model_deinit(struct model *model) {
#define release(ptr)                                                           \
    if (ptr != NULL) {                                                         \
        free(ptr);                                                             \
        ptr = NULL;                                                            \
    }

    release(model->vert_x);
    release(model->vert_y);
    release(model->vert_z);
    release(model->vert_shade);
    release(model->vert_ambient);

    release(model->face_vert_cnt);
    for (size_t i = 0; i < model->face_cnt; i++) {
        free(model->face_verts[i]);
    }
    release(model->face_verts);
    release(model->texture_front);
    release(model->texture_back);
    release(model->light_type);
    release(model->norm_scale);
    release(model->norm_mag);

    release(model->project_vert_x);
    release(model->project_vert_y);
    release(model->project_vert_z);
    release(model->project_plane_x);
    release(model->project_plane_y);

    release(model->is_local);
    release(model->face_pick_tag);

    release(model->trans_vert_x);
    release(model->trans_vert_y);
    release(model->trans_vert_z);

    release(model->face_norm_x);
    release(model->face_norm_y);
    release(model->face_norm_z);

    release(model->face_min_x);
    release(model->face_max_x);
    release(model->face_min_y);
    release(model->face_max_y);
    release(model->face_min_z);
    release(model->face_max_z);

#undef release
}

static void allocate(struct model *model, size_t vert_cnt, size_t face_cnt) {
#define init(ptr, size) ptr = realloc(ptr, size)

    model->max_vert_cnt = vert_cnt;
    model->vert_cnt = 0;
    init(model->vert_x, sizeof(int16_t) * vert_cnt);
    init(model->vert_y, sizeof(int16_t) * vert_cnt);
    init(model->vert_z, sizeof(int16_t) * vert_cnt);
    init(model->vert_shade, sizeof(int32_t) * vert_cnt);
    init(model->vert_ambient, sizeof(int32_t) * vert_cnt);

    model->max_face_cnt = face_cnt;
    model->face_cnt = 0;
    init(model->face_vert_cnt, sizeof(uint8_t) * face_cnt);
    init(model->face_verts, sizeof(int16_t *) * face_cnt);
    init(model->texture_front, sizeof(int32_t) * face_cnt);
    init(model->texture_back, sizeof(int32_t) * face_cnt);
    init(model->light_type, sizeof(uint16_t) * face_cnt);
    init(model->norm_scale, sizeof(int32_t) * face_cnt);
    init(model->norm_mag, sizeof(int32_t) * face_cnt);

    if (!model->unrendered) {
        init(model->project_vert_x, sizeof(int32_t) * vert_cnt);
        init(model->project_vert_y, sizeof(int32_t) * vert_cnt);
        init(model->project_vert_z, sizeof(int32_t) * vert_cnt);
        init(model->project_plane_x, sizeof(int32_t) * vert_cnt);
        init(model->project_plane_y, sizeof(int32_t) * vert_cnt);
    }

    if (!model->unpickable) {
        init(model->is_local, sizeof(uint8_t) * face_cnt);
        init(model->face_pick_tag, sizeof(uint32_t) * face_cnt);
    }

    if (model->autocommit) {
        model->trans_vert_x = model->vert_x;
        model->trans_vert_y = model->vert_y;
        model->trans_vert_z = model->vert_z;
    } else {
        init(model->trans_vert_x, sizeof(int16_t) * vert_cnt);
        init(model->trans_vert_y, sizeof(int16_t) * vert_cnt);
        init(model->trans_vert_z, sizeof(int16_t) * vert_cnt);
    }

    if (!model->no_shading || !model->no_bounds) {
        init(model->face_norm_x, sizeof(int32_t) * face_cnt);
        init(model->face_norm_y, sizeof(int32_t) * face_cnt);
        init(model->face_norm_z, sizeof(int32_t) * face_cnt);
    }

    if (!model->no_bounds) {
        init(model->face_min_x, sizeof(int32_t) * face_cnt);
        init(model->face_max_x, sizeof(int32_t) * face_cnt);
        init(model->face_min_y, sizeof(int32_t) * face_cnt);
        init(model->face_max_y, sizeof(int32_t) * face_cnt);
        init(model->face_min_z, sizeof(int32_t) * face_cnt);
        init(model->face_max_z, sizeof(int32_t) * face_cnt);
    }

    model->light_x = 180;
    model->light_y = 155;
    model->light_z = 95;
    model->light_mag = 256;
    model->light_falloff = 512;
    model->light_ambient = 32;
    model->scale_x = model->scale_y = model->scale_z = 256;
    model->shear_xy = model->shear_zy = model->shear_xz = 256;
    model->shear_yz = model->shear_zx = model->shear_yx = 256;
    model->trans_type = 0;

#undef init
}

void model_project_prepare(struct model *model) {
#define init(ptr, size) ptr = realloc(ptr, size)

    init(model->project_vert_x, sizeof(int32_t) * model->vert_cnt);
    init(model->project_vert_y, sizeof(int32_t) * model->vert_cnt);
    init(model->project_vert_z, sizeof(int32_t) * model->vert_cnt);
    init(model->project_plane_x, sizeof(int32_t) * model->vert_cnt);
    init(model->project_plane_y, sizeof(int32_t) * model->vert_cnt);

#undef init
}

void model_clear(struct model *model) {
    model->vert_cnt = 0;
    model->face_cnt = 0;
}

void model_reduce(struct model *model, size_t face_cnt, size_t vert_cnt) {
    model->face_cnt = sat_subsz(model->face_cnt, face_cnt);
    model->vert_cnt = sat_subsz(model->vert_cnt, vert_cnt);
}

void model_join(struct model *model, struct model *parts, size_t size) {
    size_t vert_cnt = 0;
    size_t face_cnt = 0;

    for (size_t i = 0; i < size; i++) {
        struct model *part = &parts[i];
    }
}

size_t model_vert_add(struct model *model, int16_t x, int16_t y, int16_t z) {
    if (model->vert_cnt >= model->max_vert_cnt) {
        return 0;
    }

    model->vert_x[model->vert_cnt] = x;
    model->vert_y[model->vert_cnt] = y;
    model->vert_z[model->vert_cnt] = z;
    return model->vert_cnt++;
}

size_t model_vert_get(struct model *model, int16_t x, int16_t y, int16_t z) {
    for (size_t i = 0; i < model->vert_cnt; i++) {
        if (model->vert_x[i] == x && model->vert_y[i] == y &&
            model->vert_z[i] == z) {
            return i;
        }
    }

    return model_vert_add(model, x, y, z);
}

size_t model_face_add(struct model *model, size_t vert_cnt, int16_t *verts,
                      int32_t front, int32_t back) {
    if (model->face_cnt >= model->max_face_cnt) {
        return 0;
    }
    model->face_vert_cnt[model->face_cnt] = vert_cnt;
    model->face_verts[model->face_cnt] = malloc(sizeof(int16_t) * vert_cnt);
    memcpy(model->face_verts[model->face_cnt], verts,
           sizeof(int16_t) * vert_cnt);
    model->texture_front[model->face_cnt] = front;
    model->texture_back[model->face_cnt] = back;
    return model->face_cnt++;
}

void model_scale(struct model *model, int32_t x, int32_t y, int32_t z) {
    model->scale_x = x;
    model->scale_y = y;
    model->scale_z = z;
    determine_trans(model);
    model->trans_state = 1;
}

void model_shear(struct model *model, int32_t xy, int32_t zy, int32_t xz,
                 int32_t yz, int32_t zx, int32_t yx) {
    model->shear_xy = xy;
    model->shear_zy = zy;
    model->shear_xz = xz;
    model->shear_yz = yz;
    model->shear_zx = zx;
    model->shear_yx = yx;
    determine_trans(model);
    model->trans_state = 1;
}

void model_rotate(struct model *model, uint8_t yaw, uint8_t pitch,
                  uint8_t roll) {
    model->yaw = model->yaw + (yaw & 0xFF);
    model->pitch = model->pitch + (pitch & 0xFF);
    model->roll = model->roll + (roll & 0xFF);
    determine_trans(model);
    model->trans_state = 1;
}

void model_orient(struct model *model, uint8_t yaw, uint8_t pitch,
                  uint8_t roll) {
    model->yaw = (yaw & 0xFF);
    model->pitch = (pitch & 0xFF);
    model->roll = (roll & 0xFF);
    determine_trans(model);
    model->trans_state = 1;
}

void model_translate(struct model *model, int32_t x, int32_t y, int32_t z) {
    model->base_x += x;
    model->base_y += y;
    model->base_z += z;
    determine_trans(model);
    model->trans_state = 1;
}

void model_position(struct model *model, int32_t x, int32_t y, int32_t z) {
    model->base_x = x;
    model->base_y = y;
    model->base_z = z;
    determine_trans(model);
    model->trans_state = 1;
}

static void determine_trans(struct model *model) {
    if ((model->shear_xy != 256) || (model->shear_zy != 256) ||
        (model->shear_xz != 256) || (model->shear_yz != 256) ||
        (model->shear_zx != 256) || (model->shear_yx != 256)) {
        model->trans_type = 4;
        return;
    }
    if ((model->scale_x != 256) || (model->scale_y != 256) ||
        (model->scale_z != 256)) {
        model->trans_type = 3;
        return;
    }
    if ((model->yaw != 0) || (model->pitch != 0) || (model->roll != 0)) {
        model->trans_type = 2;
        return;
    }
    if ((model->base_x != 0) || (model->base_y != 0) || (model->base_z != 0)) {
        model->trans_type = 1;
        return;
    }
    model->trans_type = 0;
}

void model_commit(struct model *model) {
    model_trans_apply(model);

    memcpy(model->vert_x, model->trans_vert_x,
           sizeof(int16_t) * model->vert_cnt);
    memcpy(model->vert_y, model->trans_vert_y,
           sizeof(int16_t) * model->vert_cnt);
    memcpy(model->vert_z, model->trans_vert_z,
           sizeof(int16_t) * model->vert_cnt);

    model->base_x = model->base_y = model->base_z = 0;
    model->yaw = model->pitch = model->roll = 0;
    model->scale_x = model->scale_y = model->scale_z = 256;
    model->shear_xy = model->shear_zy = model->shear_xz = 256;
    model->shear_yz = model->shear_zx = model->shear_yx = 256;
    model->trans_type = 0;
}

void model_trans_apply(struct model *model) {
    if (model->trans_state == 2) {
        model->trans_state = 0;

        memcpy(model->trans_vert_x, model->vert_x,
               sizeof(int16_t) * model->vert_cnt);
        memcpy(model->trans_vert_y, model->vert_y,
               sizeof(int16_t) * model->vert_cnt);
        memcpy(model->trans_vert_z, model->vert_z,
               sizeof(int16_t) * model->vert_cnt);

        model->min_x = model->min_y = model->min_z = -9999999;
        model->max_dim = model->max_x = model->max_y = model->max_z = -9999999;
    } else if (model->trans_state == 1) {
        model->trans_state = 0;

        memcpy(model->trans_vert_x, model->vert_x,
               sizeof(int16_t) * model->vert_cnt);
        memcpy(model->trans_vert_y, model->vert_y,
               sizeof(int16_t) * model->vert_cnt);
        memcpy(model->trans_vert_z, model->vert_z,
               sizeof(int16_t) * model->vert_cnt);

        if (model->trans_type >= 2) {
            rotate_apply(model);
        }
        if (model->trans_type >= 3) {
            scale_apply(model);
        }
        if (model->trans_type >= 4) {
            shear_apply(model);
        }
        if (model->trans_type >= 1) {
            translate_apply(model);
        }

        calc_bounds(model);
        model_calc_norms(model);
    }
}

void rotate_apply(struct model *model) {
    for (size_t i = 0; i < model->vert_cnt; ++i) {
        if (model->roll != 0) {
            int32_t sin = sin256_tbl[model->roll];
            int32_t cos = sin256_tbl[model->roll + 256];
            int16_t vert = ((model->trans_vert_y[i] * sin) +
                            (model->trans_vert_x[i] * cos)) >>
                           15;

            model->trans_vert_y[i] = ((model->trans_vert_y[i] * cos) -
                                      (model->trans_vert_x[i] * sin)) >>
                                     15;
            model->trans_vert_x[i] = vert;
        }

        if (model->yaw != 0) {
            int32_t sin = sin256_tbl[model->yaw];
            int32_t cos = sin256_tbl[model->yaw + 256];
            int16_t vert = ((model->trans_vert_y[i] * cos) -
                            (model->trans_vert_z[i] * sin)) >>
                           15;
            model->trans_vert_z[i] = ((model->trans_vert_y[i] * sin) +
                                      (model->trans_vert_z[i] * cos)) >>
                                     15;
            model->trans_vert_y[i] = vert;
        }

        if (model->pitch != 0) {
            int sin = sin256_tbl[model->pitch];
            int cos = sin256_tbl[model->pitch + 256];
            int vert = ((model->trans_vert_z[i] * sin) +
                        (model->trans_vert_x[i] * cos)) >>
                       15;
            model->trans_vert_z[i] = ((model->trans_vert_z[i] * cos) -
                                      (model->trans_vert_x[i] * sin)) >>
                                     15;
            model->trans_vert_x[i] = vert;
        }
    }
}

void scale_apply(struct model *model) {}

void shear_apply(struct model *model) {}

void translate_apply(struct model *model) {}

void calc_bounds(struct model *model) {}

void model_calc_norms(struct model *model) {}
