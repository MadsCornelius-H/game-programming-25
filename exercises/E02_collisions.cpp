#define STB_IMAGE_IMPLEMENTATION

#define ITU_UNITY_BUILD


#include <SDL3/SDL.h>
#include <stb_image.h>

#include <itu_common.hpp>
#include <itu_lib_render.hpp>
#include <itu_lib_overlaps.hpp>

#define ENABLE_DIAGNOSTICS

#define TARGET_FRAMERATE SECONDS(1) / 60
#define WINDOW_W 1200
#define WINDOW_H 1200
#define WINDOW_MID_W WINDOW_W / 2
#define WINDOW_MID_H WINDOW_H / 2

#define ENTITY_COUNT 4096
#define ASTEROIDS_CREATED 2300
#define MAX_COLLISIONS 1024   // num max collisions per frame

bool DEBUG_separate_collisions   = true;
bool DEBUG_render_colliders      = true;
bool DEBUG_render_texture_border = false;

struct Entity;
struct EntityCollisionInfo;

struct SDLContext
{
	SDL_Renderer* renderer;
	float zoom;     // render zoom
	float window_w;	// current window width after render zoom has been applied
	float window_h;	// current window width after render zoom has been applied

	float delta;    // in seconds
	float uptime;   // in seconds

	bool btn_isdown_up;
	bool btn_isdown_down;
	bool btn_isdown_left;
	bool btn_isdown_right;
	bool btn_isdown_space;
};

struct GameState
{
	Entity* player;
	Entity* entities;

	// game-allocated memory
	// Entity* entities;
	int entities_alive_count;

	// location based separation
	Entity** topleft_quadrant;
	Entity** topright_quadrant;
	Entity** bottomleft_quadrant;
	Entity** bottomright_quadrant;

	int topleft_count;
    int topright_count;
    int bottomleft_count;
    int bottomright_count;

	EntityCollisionInfo* frame_collisions;
	int frame_collisions_count;

	// SDL-allocated structures
	SDL_Texture* atlas;
};

static SDL_Texture* texture_create(SDLContext* context, const char* path)
{
	int w=0, h=0, n=0;
	unsigned char* pixels = stbi_load(path, &w, &h, &n, 0);
	SDL_Surface* surface = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_ABGR8888, pixels, w * n);

	SDL_Texture* ret = SDL_CreateTextureFromSurface(context->renderer, surface);

	SDL_DestroySurface(surface);
	stbi_image_free(pixels);

	return ret;
}

// ********************************************************************************************************************
// sprite
// ********************************************************************************************************************

struct Sprite
{
	SDL_Texture* texture;
	SDL_FRect    rect;
	color        tint;
	vec2f        pivot;
};

// quick sprite rendering function that takes care of most of the functionalities
// NOTE: this function is still temporary since ATM we can't really deal with game worlds bigger than the rendering window
//       we will address it in lecture 03, and then we will just create a final sprite system and be done with it
static void sprite_render(SDLContext* context, vec2f position, vec2f size, Sprite* sprite)
{
	SDL_FRect dst_rect;
	dst_rect.w = size.x;
	dst_rect.h = size.y;
	dst_rect.x = position.x - dst_rect.w * sprite->pivot.x;
	dst_rect.y = position.y - dst_rect.h * sprite->pivot.y;
		
	SDL_SetTextureColorModFloat(sprite->texture, sprite->tint.r, sprite->tint.g, sprite->tint.b);
	SDL_SetTextureAlphaModFloat(sprite->texture, sprite->tint.a);
	SDL_RenderTexture(context->renderer, sprite->texture, &sprite->rect, &dst_rect);

	if(DEBUG_render_texture_border)
	{
		SDL_SetRenderDrawColorFloat(context->renderer, 1, 1, 1, 1);
		SDL_RenderRect(context->renderer, &dst_rect);
	}
}

// ********************************************************************************************************************
// entity
// ********************************************************************************************************************

struct Entity
{
	vec2f position;
	vec2f size;

	Sprite sprite;

	// collider info
	bool is_static;
	vec2f min_vals;
	vec2f max_vals;
	float collider_radius;
	vec2f collider_offset;
};

static Entity* entity_create(GameState* state)
{
	if(!(state->entities_alive_count < ENTITY_COUNT))
		return NULL;

	// // concise version
	//return &state->entities[state->entities_alive_count++];


	Entity* ret = &state->entities[state->entities_alive_count];
	++state->entities_alive_count;

	return ret;
}

// NOTE: this only works if nobody holds references to other entities!
// static void entity_destroy(GameState* state, Entity* entity)
// {
// 	// NOTE: here we want to fail hard, nobody should pass us a pointer not gotten from `entity_create()`
// 	SDL_assert(entity < state->entities || entity > state->entities + ENTITY_COUNT);

// 	--state->entities_alive_count;
// 	*entity = state->entities[state->entities_alive_count];
// }
// ********************************************************************************************************************
// collisions
// ********************************************************************************************************************

struct EntityCollisionInfo
{
	Entity* e1;
	Entity* e2;

	vec2f normal;
	float separation;
};

static void quadrant_collisions(GameState* state, Entity** quadrant, int count, int colour_nr){

	for(int i = 0; i < count - 1; ++i)
	{
		Entity* e1 = quadrant[i];
		
		for(int j = i + 1; j < count; ++j)
		{
			Entity* e2 = quadrant[j];
			if (e1->is_static && e2->is_static) continue;
			if(itu_lib_overlaps_circle_circle(
				e1->position + e1->collider_offset, e1->collider_radius,
				e2->position + e2->collider_offset, e2->collider_radius
			))
			{
				if (colour_nr == 0){
					e1->sprite.tint = COLOR_RED;
					e2->sprite.tint = COLOR_RED;
				}
				if (colour_nr == 1){
					e1->sprite.tint = COLOR_BLUE;
					e2->sprite.tint = COLOR_BLUE;
				}
				if (colour_nr == 2){
					e1->sprite.tint = COLOR_GREEN;
					e2->sprite.tint = COLOR_GREEN;
				}
				if (colour_nr == 3){
					e1->sprite.tint = COLOR_YELLOW;
					e2->sprite.tint = COLOR_YELLOW;
				}

				if(state->frame_collisions_count >= MAX_COLLISIONS)
				{
					SDL_Log("[WARNING] too many collisions!");
					return;
				}

				// NOTE: here we are redoing a bunch of work that we already done in the overlap test. An easy optimization is do to have the test return the collision info
				vec2f v = (e2->position + e2->collider_offset) - (e1->position + e1->collider_offset);
				float l = length(v);
				float separation_vector = e1->collider_radius + e2->collider_radius - l;
				int new_collision_idx = state->frame_collisions_count++;

				state->frame_collisions[new_collision_idx].e1 = e1;
				state->frame_collisions[new_collision_idx].e2 = e2;
				state->frame_collisions[new_collision_idx].normal = v / l; // normalize vector (we already need the length, so we don't need to call normalize which would do that anyway)
				state->frame_collisions[new_collision_idx].separation = separation_vector;
			}
		}
	}
}

static void collision_check(GameState* state)
{
	state->frame_collisions_count = 0;
	quadrant_collisions(state, state->topleft_quadrant, state->topleft_count, 0);
	quadrant_collisions(state, state->topright_quadrant, state->topright_count, 1);
	quadrant_collisions(state, state->bottomleft_quadrant, state-> bottomleft_count, 2);
	quadrant_collisions(state, state->bottomright_quadrant, state->bottomright_count, 3);	
}

static void collision_separate(GameState* state)
{
	for(int i = 0; i < state->frame_collisions_count; ++i)
	{
		EntityCollisionInfo entity_collision_info = state->frame_collisions[i];

		vec2f sep = entity_collision_info.normal * entity_collision_info.separation / 2;
		entity_collision_info.e1->position -= sep;
		entity_collision_info.e2->position += sep;
	}
}

// ********************************************************************************************************************
// game
// ********************************************************************************************************************

static void game_init(SDLContext* context, GameState* state)
{
	// contiguous memory
	{
		state->entities = (Entity*)SDL_malloc(ENTITY_COUNT * sizeof(Entity));
		SDL_assert(state->entities);

		state->frame_collisions = (EntityCollisionInfo*)SDL_malloc(MAX_COLLISIONS * sizeof(EntityCollisionInfo));
		SDL_assert(state->frame_collisions);

		state->topleft_quadrant = (Entity**)SDL_malloc(ENTITY_COUNT * sizeof(Entity*));
		state->topright_quadrant = (Entity**)SDL_malloc(ENTITY_COUNT * sizeof(Entity*));
		state->bottomleft_quadrant = (Entity**)SDL_malloc(ENTITY_COUNT * sizeof(Entity*));
		state->bottomright_quadrant = (Entity**)SDL_malloc(ENTITY_COUNT * sizeof(Entity*));

		state->topleft_count = state->topright_count = 0;
		state->bottomleft_count = state->bottomright_count = 0;
	}

	// texture atlasesw
	state->atlas = texture_create(context, "data/kenney/simpleSpace_tilesheet_2.png");

}

static void game_reset(SDLContext* context, GameState* state)
{
	SDL_memset(state->entities, 0, ENTITY_COUNT * sizeof(Entity));
	state->entities_alive_count = 0;

	SDL_memset(state->topleft_quadrant, 0, ENTITY_COUNT * sizeof(Entity*));
	SDL_memset(state->topright_quadrant, 0, ENTITY_COUNT * sizeof(Entity*));
	SDL_memset(state->bottomleft_quadrant, 0, ENTITY_COUNT * sizeof(Entity*));
	SDL_memset(state->bottomright_quadrant, 0, ENTITY_COUNT * sizeof(Entity*));

	// entities
	Entity* player = entity_create(state);
	// we always have a player. This should also always be the first entity created, so it should never fail
	SDL_assert(player);
	player->position.x = (float)context->window_w / 4;
    player->position.y = (float)context->window_h / 4;
	player->size = vec2f{ 64, 64 };
	player->is_static = false;
	player->sprite = {
		.texture = state->atlas,
		.rect = SDL_FRect{ 0, 0, 128, 128 },
		.tint = COLOR_WHITE,
		.pivot = vec2f{ 0.5f, 0.5f }
	};
	player->collider_radius = 16;
	player->min_vals.x = player->collider_radius;
	player->min_vals.y = player->collider_radius;
	player->max_vals.x = WINDOW_W-player->collider_radius;
	player->max_vals.y = WINDOW_H-player->collider_radius;
	state->player = player;

	// grid pattern
	for(int i = 0; i < ASTEROIDS_CREATED; ++i)
	{
		Entity* entity = entity_create(state);
		if(!entity)
		{
			SDL_Log("[WARNING] too many entity spawned!");
			break;
		}
		
		vec2f coords = vec2f{ 5.5f + i % 10, 5.5f + i /10};
		entity->size = vec2f{ 32, 32 };
		entity->is_static = false;
		entity->position = mul_element_wise(entity->size,  coords);
		entity->sprite = {
			.texture = state->atlas,
			.rect = SDL_FRect{ 0, 4*128, 128, 128 },
			.tint = COLOR_WHITE,
			.pivot = vec2f{ 0.5f, 0.5f }
		};
		entity->collider_radius = 16;
		entity->min_vals.x = entity->collider_radius;
		entity->min_vals.y = entity->collider_radius;
		entity->max_vals.x = WINDOW_W-entity->collider_radius;
		entity->max_vals.y = WINDOW_H-entity->collider_radius;
	}
}

static void game_update(SDLContext* context, GameState* state)
{
	state->topleft_count = 0;
	state->topright_count = 0;
	state->bottomleft_count = 0;
	state->bottomright_count = 0;

	vec2f mov = { 0 };
	if(context->btn_isdown_up)
		mov.y -= 1;
	if(context->btn_isdown_down)
		mov.y += 1;
	if(context->btn_isdown_left)
		mov.x -= 1;
	if(context->btn_isdown_right)
		mov.x += 1;

	vec2f velocity = normalize(mov) * (128 * context->delta);
	state->player->position = state->player->position + velocity;

	// reset tint and make sure entity doesn't exit window
	for(int i = 0; i < state->entities_alive_count; ++i)
	{
		Entity* entity = &state->entities[i];

		// Entity* entity = &state->entities[i];
		entity->sprite.tint = COLOR_WHITE;

		// Make sure object cant leave screen
		vec2f position = entity->position;
		if (position.x < entity->min_vals.x) entity->position.x = entity->min_vals.x;
		if (position.x > entity->max_vals.x) entity->position.x = entity->max_vals.x;
		if (position.y < entity->min_vals.y) entity->position.y = entity->min_vals.y;
		if (position.y > entity->max_vals.y) entity->position.y = entity->max_vals.y;

		float left   = entity->position.x - entity->collider_radius;
		float right  = entity->position.x + entity->collider_radius;
		float top    = entity->position.y - entity->collider_radius;
		float bottom = entity->position.y + entity->collider_radius;

		bool in_left   = (left   < WINDOW_MID_W);
		bool in_right  = (right  > WINDOW_MID_W);
		bool in_top    = (top    < WINDOW_MID_H);
		bool in_bottom = (bottom > WINDOW_MID_H);

		if (in_left && in_top)
		    state->topleft_quadrant[state->topleft_count++] = entity;

		if (in_right && in_top)
		    state->topright_quadrant[state->topright_count++] = entity;

		if (in_left && in_bottom)
		    state->bottomleft_quadrant[state->bottomleft_count++] = entity;

		if (in_right && in_bottom)
		    state->bottomright_quadrant[state->bottomright_count++] = entity;

	}

	collision_check(state);
	if(DEBUG_separate_collisions)
		collision_separate(state);
}

static void game_render(SDLContext* context, GameState* state)
{
	// render
	for(int i = 0; i < state->entities_alive_count; ++i)
	{
		Entity* entity = &state->entities[i];
		sprite_render(context, entity->position, entity->size, &entity->sprite);

		if(DEBUG_render_colliders)
		{
			itu_lib_render_draw_point(context->renderer, entity->position + entity->collider_offset, 5, COLOR_GREEN);
			itu_lib_render_draw_circle(
				context->renderer,
				entity->position + entity->collider_offset,
				entity->collider_radius,
				16, COLOR_GREEN
			);
		}
	}
		
	// debug window
	SDL_SetRenderDrawColor(context->renderer, 0xFF, 0x00, 0xFF, 0xff);
	SDL_RenderRect(context->renderer, NULL);
}

int main(void)
{
	int a = sizeof(int*);
	bool quit = false;
	SDL_Window* window;
	SDLContext context = { 0 };
	GameState  state   = { 0 };

	context.window_w = WINDOW_W;
	context.window_h = WINDOW_H;

	SDL_CreateWindowAndRenderer("E02 - Collisions", context.window_w, context.window_h, 0, &window, &context.renderer);

	SDL_SetRenderDrawBlendMode(context.renderer, SDL_BLENDMODE_BLEND);
	
	// increase the zoom to make debug text more legible
	// (ie, on the class projector, we will usually use 2)
	{
		context.zoom = 1;
		context.window_w /= context.zoom;
		context.window_h /= context.zoom;
		SDL_SetRenderScale(context.renderer, context.zoom, context.zoom);
	}

	game_init(&context, &state);
	game_reset(&context, &state);

	SDL_Time walltime_frame_beg;
	SDL_Time walltime_frame_end;
	SDL_Time walltime_work_end;
	SDL_Time elapsed_work;
	SDL_Time elapsed_frame;

	SDL_GetCurrentTime(&walltime_frame_beg);
	walltime_frame_end = walltime_frame_beg;

	while(!quit)
	{
		// input
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_EVENT_QUIT:
					quit = true;
					break;
					
				case SDL_EVENT_KEY_DOWN:
				case SDL_EVENT_KEY_UP:
					switch(event.key.key)
					{
						case SDLK_W: context.btn_isdown_up    = event.key.down; break;
						case SDLK_A: context.btn_isdown_left  = event.key.down; break;
						case SDLK_S: context.btn_isdown_down  = event.key.down; break;
						case SDLK_D: context.btn_isdown_right = event.key.down; break;
						case SDLK_SPACE: context.btn_isdown_space = event.key.down; break;
					}

					// debug keys
					if(event.key.down && !event.key.repeat)
					{
						switch(event.key.key)
						{
							case SDLK_TAB: game_reset(&context, &state); break;
							case SDLK_F1: DEBUG_separate_collisions   = !DEBUG_separate_collisions;   break;
							case SDLK_F2: DEBUG_render_colliders      = !DEBUG_render_colliders;      break;
							case SDLK_F3: DEBUG_render_texture_border = !DEBUG_render_texture_border; break;
						}
					}
					break;
			}
		}

		SDL_SetRenderDrawColor(context.renderer, 0x00, 0x00, 0x00, 0x00);
		SDL_RenderClear(context.renderer);

		// update
		game_update(&context, &state);
		game_render(&context, &state);

		SDL_GetCurrentTime(&walltime_work_end);
		elapsed_work = walltime_work_end - walltime_frame_beg;

		if(elapsed_work < TARGET_FRAMERATE)
			SDL_DelayPrecise(TARGET_FRAMERATE - elapsed_work);
		SDL_GetCurrentTime(&walltime_frame_end);
		elapsed_frame = walltime_frame_end - walltime_frame_beg;
		

#ifdef ENABLE_DIAGNOSTICS
		{
			SDL_SetRenderDrawColor(context.renderer, 0x0, 0x00, 0x00, 0xCC);
			SDL_FRect rect = SDL_FRect{ 5, 5, 225, 65 };
			SDL_RenderFillRect(context.renderer, &rect);
			SDL_SetRenderDrawColor(context.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			SDL_RenderDebugTextFormat(context.renderer, 10, 10, "work: %9.6f ms/f", (float)elapsed_work  / (float)MILLIS(1));
			SDL_RenderDebugTextFormat(context.renderer, 10, 20, "tot : %9.6f ms/f", (float)elapsed_frame / (float)MILLIS(1));
			SDL_RenderDebugTextFormat(context.renderer, 10, 30, "[TAB] reset ");
			SDL_RenderDebugTextFormat(context.renderer, 10, 40, "[F1]  collisions        %s", DEBUG_separate_collisions   ? " ON" : "OFF");
			SDL_RenderDebugTextFormat(context.renderer, 10, 50, "[F2]  render colliders  %s", DEBUG_render_colliders      ? " ON" : "OFF");
			SDL_RenderDebugTextFormat(context.renderer, 10, 60, "[F3]  render tex border %s", DEBUG_render_texture_border ? " ON" : "OFF");
		}
#endif

		// render
		SDL_RenderPresent(context.renderer);

		context.delta = (float)elapsed_frame / (float)SECONDS(1);
		context.uptime += context.delta;
		walltime_frame_beg = walltime_frame_end;
	}
}