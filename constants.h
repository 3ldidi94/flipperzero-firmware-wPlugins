#define TAG "SpaceImpact"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define HERO_SIZE 8

#define HERO_MIN_Y 7
#define HERO_MAX_Y SCREEN_HEIGHT - HERO_SIZE

#define BULLET_PULL 25

#define membersof(x) (sizeof(x) / sizeof(x[0]))

typedef enum {
    EventTypeTick,
    EventTypeInput,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} GameEvent;

typedef struct {
    uint8_t x;
    uint8_t y;
} Vector2;

typedef struct {
    int score;
    int life;
    int rockets;
    Vector2 bullets[BULLET_PULL];
    Vector2 position;
} PlayerState;

typedef struct {
    PlayerState player;

    bool hasB;
    bool fire;
    bool rocket;
    bool up;
    bool down;
} GameState;

void game_init(GameState* const state) {
    state->player.life = 3;
    state->player.rockets = 3;
    state->player.score = 0;

    state->player.position.x = 1;
    state->player.position.y = (HERO_MAX_Y) / 2;
}