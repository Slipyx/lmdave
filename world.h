// world/level state
#ifndef __WORLD_H
#define __WORLD_H

void W_StartLevel();
void W_ResetLevel();
uint8_t W_IsClear( uint16_t px, uint16_t py, uint8_t is_player );
void W_Update();
void W_ScrollView();

#endif
