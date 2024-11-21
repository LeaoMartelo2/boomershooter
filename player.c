#include "player.h"
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

bool check_colision_test(Player *player, Vector3 floor_pos, Vector3 floor_size) {

    Vector3 entity_pos = player->postition;
    Vector3 entity_size = player->bounding_box_size;

    Vector3 negative = {entity_pos.x - entity_size.x / 2,
                        entity_pos.y - entity_size.y / 2,
                        entity_pos.z - entity_size.z / 2};

    Vector3 positive = {entity_pos.x + entity_size.x / 2,
                        entity_pos.y + entity_size.y / 2,
                        entity_pos.z + entity_size.z / 2};

    BoundingBox entity_box = (BoundingBox){negative, positive};

    /*Vector3 floor_size = (Vector3){600, 10, 600};*/

    Vector3 neg = {floor_pos.x - floor_size.x / 2,
                   floor_pos.y - floor_size.y / 2,
                   floor_pos.z - floor_size.z / 2};

    Vector3 pos = {floor_pos.x + floor_size.x / 2,
                   floor_pos.y + floor_size.y / 2,
                   floor_pos.z + floor_size.z / 2};

    BoundingBox floor_box = (BoundingBox){neg, pos};

    if (CheckCollisionBoxes(entity_box, floor_box)) {

        DrawBoundingBox(floor_box, GREEN);
        DrawBoundingBox(entity_box, GREEN);

        return true;
    }

    return false;
}

void set_camera_pos(Player *player) {
    player->camera.position = player->postition;
    player->camera.position.y = player->postition.y + 9;
    player->camera.up.y = 1.0f;
    player->camera.target = player->camera.target;
}

void player_move_forward(Player *player, float distance) {
    /* direction its facing */
    Vector3 forward = Vector3Normalize(Vector3Subtract(player->camera.target, player->camera.position));

    /* project vector in to world plane */
    forward.y = 0;
    forward = Vector3Normalize(forward);

    /* scale by distance */
    forward = Vector3Scale(forward, distance);

    player->postition = Vector3Add(player->postition, forward);
    /*player->camera.position = player->postition;*/
    set_camera_pos(player);
    player->camera.target = Vector3Add(player->camera.target, forward);
}

void player_move_right(Player *player, float distance) {

    Vector3 forward = Vector3Normalize(Vector3Subtract(player->camera.target, player->camera.position));

    Vector3 up = Vector3Normalize(player->camera.up);

    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, up));

    right.y = 0;
    right = Vector3Scale(right, distance);

    player->postition = Vector3Add(player->postition, right);
    /*player->camera.position = player->postition;*/
    set_camera_pos(player);
    player->camera.target = (Vector3Add(player->camera.target, right));
}

void player_move_vertical(Player *player, float distance) {

    Vector3 up = Vector3Normalize(player->camera.up);

    up = Vector3Scale(up, distance);

    player->postition = Vector3Add(player->postition, up);
    set_camera_pos(player);
    player->camera.target = Vector3Add(player->camera.target, up);
}

void clamp_float(float *velocity) {
    if (*velocity > 1) {
        *velocity = 1;
    }

    if (*velocity < -1) {
        *velocity = -1;
    }

    // deadzone to avoid floating point errors:

    float dead_zone = 0.1;

    if (fabsf(*velocity) < dead_zone) {
        *velocity = 0.0f;
    }
}

void update_velocity(float *velocity, float decay_rate) {

    if (*velocity > 0.0000f) {
        *velocity -= decay_rate;
    }

    if (*velocity < 0.0000f) {
        *velocity += decay_rate;
    }
}

void update_gravity(float *velocity, float gravity, float rate) {

    if (*velocity > gravity) {
        *velocity -= rate;
    }
}

void move_cam(Player *p) {

    float delta_time = GetFrameTime();
    Vector2 mouse_pos_delta = GetMouseDelta();

    float decay_rate = 0.1f;

    printf("velocities: {%f, %f}\n", p->forward_velocity, p->sideways_velocity);

    update_velocity(&p->forward_velocity, decay_rate);
    update_velocity(&p->sideways_velocity, decay_rate);

    clamp_float(&p->forward_velocity);
    clamp_float(&p->sideways_velocity);

    Vector3 forward = GetCameraForward(&p->camera);

    CameraYaw(&p->camera, -mouse_pos_delta.x * p->cam_rot_speed * delta_time, false);
    CameraPitch(&p->camera, -mouse_pos_delta.y * p->cam_rot_speed * delta_time, true, false, false);

    player_move_forward(p, -p->move_speed * p->forward_velocity * delta_time);
    player_move_right(p, -p->move_speed * p->sideways_velocity * delta_time);

    if (IsKeyDown(KEY_W)) {
        p->forward_velocity -= 1.0f;
    }

    if (IsKeyDown(KEY_A)) {

        p->sideways_velocity += 1.0f;

        if (p->turn_A == false) {
            p->camera.up = Vector3RotateByAxisAngle(p->camera.up, forward, p->cam_rol_scale);
        }

        p->turn_A = true;
    }

    if (IsKeyReleased(KEY_A)) {
        p->camera.up = Vector3Zero();
        p->camera.up.y = 1.0f;
        p->turn_A = false;
    }

    if (IsKeyDown(KEY_S)) {
        p->forward_velocity += 1.0f;
    }

    if (IsKeyDown(KEY_D)) {

        p->sideways_velocity -= 1.0f;

        if (p->turn_D == false) {
            p->camera.up = Vector3RotateByAxisAngle(p->camera.up, forward, -p->cam_rol_scale);
        }
        p->turn_D = true;
    }
    if (IsKeyReleased(KEY_D)) {
        p->camera.up = Vector3Zero();
        p->camera.up.y = 1.0f;
        p->turn_D = false;
    }
}

void update_player(Player *player) {

    float delta_time = GetFrameTime();

    /*player->vertical_velocity = player->gravity;*/

    /*update_velocity(&player->vertical_velocity, 1.0f);*/
    update_gravity(&player->vertical_velocity, player->gravity, 15.0f);

    if (check_colision_test(player, Vector3Zero(), (Vector3){600, 10, 600})) {
        player->is_grounded = true;

    } else {
        player_move_vertical(player, player->vertical_velocity * delta_time);
    }

    if (IsKeyDown(KEY_SPACE)) {
        if (player->is_grounded) {
            player->vertical_velocity = 250;

            player_move_vertical(player, player->vertical_velocity * delta_time);
            player->is_grounded = false;
        }
    }
}
