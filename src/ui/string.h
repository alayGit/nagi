#ifndef NAGI_UI_STRING_H
#define NAGI_UI_STRING_H

extern u16 string_to_int(u8 *string);
extern u8 *int_to_string(u16 num);
extern u8 *string_zero_pad(u8 *str, u16 pad_size);
extern u8 *int_to_hex_string(u16 num);
extern u8 *string_lower(u8 *str);

#endif /* NAGI_UI_STRING_H */
