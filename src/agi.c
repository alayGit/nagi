
#include "agi.h"

	
AGI_STATE state;
AGI_STANDARD standard;

CONF_BOOL c_nagi_log_debug = 0;
CONF_BOOL c_nagi_console = 1;
CONF_BOOL c_nagi_run_correctly = 1;
CONF_BOOL c_nagi_font_benchmark = 0;
CONF_STRING c_vid_driver = 0;
CONF_INT c_vid_scale = 2;
CONF_BOOL c_vid_full_screen = 0;
CONF_INT c_vid_renderer = 0;
CONF_STRING c_vid_pal_16 = 0;
CONF_STRING c_vid_pal_text = 0;
CONF_STRING c_vid_pal_cga0 = 0;
CONF_STRING c_vid_pal_cga1 = 0;
CONF_STRING c_vid_pal_bw = 0;
CONF_STRING c_vid_fonts_bitmap = 0;
CONF_STRING c_vid_fonts_vector = 0;
CONF_STRING c_snd_driver = 0;
CONF_INT c_snd_channels = 4;
CONF_INT c_snd_gen_tone = 0;
CONF_INT c_snd_gen_noise0 = 0;
CONF_INT c_snd_gen_noise1 = 0;
CONF_BOOL c_snd_disable = 0;
CONF_STRING c_snd_sample = 0;
CONF_INT c_snd_sample_freq = 50;
CONF_STRING c_sdl_drv_video = 0;
CONF_STRING c_sdl_drv_sound = 0;

CONF_STRING c_standard_crc_list = 0;
CONF_STRING c_standard_agi_list = 0;
CONF_STRING c_standard_dir_list = 0;
CONF_STRING c_standard_force = 0;
CONF_STRING c_standard_v2_default = 0;
CONF_STRING c_standard_v3_default = 0;
CONF_STRING c_standard_sort = 0;


// for use in nagi.ini
CONF config_nagi[] =
{
	{"log_debug", "nagi", CT_BOOL, {b:{&c_nagi_log_debug, 0}} },
	{"console", 0, CT_BOOL, {b:{&c_nagi_console, 1}} },
	{"run_correctly", 0, CT_BOOL, {b:{&c_nagi_run_correctly, 1}} },
	{"font_benchmark", 0, CT_BOOL, {b:{&c_nagi_font_benchmark, 0}} },
	{"driver", "vid", CT_STRING, {s:{&c_vid_driver, "sdl"}} },
	{"scale", 0, CT_INT, {i:{&c_vid_scale, 2, 1, -1}} },
	{"full_screen", 0, CT_BOOL, {b:{&c_vid_full_screen, 0}} },
	{"renderer", 0, CT_INT, {i:{&c_vid_renderer, 0, 0, 2}} },
	{"pal_16", 0, CT_STRING, {s:{&c_vid_pal_16, "pal_16.pal"}} },
	{"pal_text", 0, CT_STRING, {s:{&c_vid_pal_text, "pal_text.pal"}} },
	{"pal_cga0", 0, CT_STRING, {s:{&c_vid_pal_cga0, "pal_cga0.pal"}} },
	{"pal_cga1", 0, CT_STRING, {s:{&c_vid_pal_cga1, "pal_cga1.pal"}} },
	{"pal_bw", 0, CT_STRING, {s:{&c_vid_pal_bw, "pal_bw.pal"}} },
	{"fonts_bitmap", 0, CT_STRING, {s:{&c_vid_fonts_bitmap, "font_4x8.nbf;font_8x8.nbf;font16x16.nbf"}} },
	{"fonts_vector", 0, CT_STRING, {s:{&c_vid_fonts_vector, "none.nvf"}} },
	{"driver", "snd", CT_STRING, {s:{&c_snd_driver, "sdl"}} },
	{"channels", 0, CT_INT, {i:{&c_snd_channels, 4, 1, 4}} },
	{"gen_tone", 0, CT_INT, {i:{&c_snd_gen_tone, 0, 0, 5}} },
	{"gen_noise0", 0, CT_INT, {i:{&c_snd_gen_noise0, 0, 0, 5}} },
	{"gen_noise1", 0, CT_INT, {i:{&c_snd_gen_noise1, 0, 0, 5}} },
	{"disable", 0, CT_BOOL, {b:{&c_snd_disable, 0}} },
	{"sample", 0, CT_STRING, {s:{&c_snd_sample, "tone50.pcm"}} },
	{"sample_freq", 0, CT_INT, {i:{&c_snd_sample_freq, 50, 0, -1}} },
	{"drv_video", "sdl", CT_STRING, {s:{&c_sdl_drv_video, "directx"}} },
	{"drv_sound", 0, CT_STRING, {s:{&c_sdl_drv_sound, "waveout"}} },
	{key: 0}
};

// for use in standard.ini
CONF config_standard[] =
{
	{"crc_list", "standard", CT_STRING, {s:{&c_standard_crc_list, "lsl"}} },
	{"agi_list", 0, CT_STRING, {s:{&c_standard_agi_list, "pc_2_936;pc_3_002_149"}} },
	{"dir_list", 0, CT_STRING, {s:{&c_standard_dir_list, "."}} },
	{"force", 0, CT_STRING, {s:{&c_standard_force, "not defined"}} },
	{"v2_default", 0, CT_STRING, {s:{&c_standard_v2_default, "pc_2_936"}} },
	{"v3_default", 0, CT_STRING, {s:{&c_standard_v3_default, "pc_3_002_149"}} },
	{"sort", 0, CT_STRING, {s:{&c_standard_sort, "alpha"}} },
	{key: 0}
};