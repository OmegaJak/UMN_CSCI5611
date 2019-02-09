#pragma once

const bool DEBUG_ON = true;
const int ELEMENTS_PER_VERT = 8;

const int ATTRIBUTE_STRIDE = ELEMENTS_PER_VERT;

const int VALUES_PER_POSITION = 3;
const int POSITION_OFFSET = 0;

const int VALUES_PER_NORMAL = 3;
const int NORMAL_OFFSET = 5;

const int VALUES_PER_TEXCOORD = 2;
const int TEXCOORD_OFFSET = 3;

const float CAMERA_ROTATION_SPEED = 0.5f;
const float CAMERA_MOVE_SPEED = 0.1f;
const float MAX_MOVE_SPEED = 10.0f * CAMERA_MOVE_SPEED;

const float ABSOLUTE_TOLERANCE = 0.00001f;
