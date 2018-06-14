#include <model.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <buffer.h>
#include <sin_table.h>

static void allocate(struct model *model, size_t vert_cnt, size_t face_cnt);

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

    for (size_t i = 0; i < vert_cnt; i++) {
        buffer_get2(&buffer, (uint16_t *)&model->vert_x[i]);
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
    init(model->vert_x, sizeof(int32_t) * vert_cnt);
    init(model->vert_y, sizeof(int32_t) * vert_cnt);
    init(model->vert_z, sizeof(int32_t) * vert_cnt);
    init(model->vert_shade, sizeof(int32_t) * vert_cnt);
    init(model->vert_ambient, sizeof(int32_t) * vert_cnt);

    model->max_face_cnt = face_cnt;
    model->face_cnt = 0;
    init(model->face_vert_cnt, sizeof(int32_t) * face_cnt);
    init(model->face_verts, sizeof(int32_t) * face_cnt);
    init(model->texture_front, sizeof(int32_t) * face_cnt);
    init(model->texture_back, sizeof(int32_t) * face_cnt);
    init(model->light_type, sizeof(int32_t) * face_cnt);
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
        init(model->trans_vert_x, sizeof(int32_t) * vert_cnt);
        init(model->trans_vert_y, sizeof(int32_t) * vert_cnt);
        init(model->trans_vert_z, sizeof(int32_t) * vert_cnt);
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
