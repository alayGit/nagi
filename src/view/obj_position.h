#ifndef NAGI_VIEW_OBJ_POSITION_H
#define NAGI_VIEW_OBJ_POSITION_H

extern u8 *cmd_position(u8 *c);
extern u8 *cmd_position_v(u8 *c);
extern u8 *cmd_get_position(u8 *c);
extern u8 *cmd_reposition(u8 *c);
extern u8 *cmd_reposition_to(u8 *c);
extern u8 *cmd_reposition_to_v(u8 *c);
extern u8 *cmd_obj_on_water(u8 *c);
extern u8 *cmd_obj_on_land(u8 *c);
extern u8 *cmd_obj_on_anything(u8 *c);
extern u8 *cmd_set_horizon(u8 *c);
extern u8 *cmd_ignore_horizon(u8 *c);
extern u8 *cmd_observe_horizon(u8 *c);

#endif /* NAGI_VIEW_OBJ_POSITION_H */
