#pragma once

#include"graphics.h"

namespace flame
{
	namespace graphics
	{
		struct FontAtlas
		{
			ImageViewPtr view;

			virtual ~FontAtlas() {}

			virtual const Glyph& get_glyph(wchar_t unicode, uint font_size) = 0;

			inline uvec2 text_offset(uint font_size, std::wstring_view str)
			{
				auto off = uvec2(0);

				for (auto ch : str)
				{
					if (ch == '\n')
					{
						off.x = 0.f;
						off.y += font_size;
					}
					else if (ch != '\r')
					{
						if (ch == '\t')
							ch = ' ';
						off.x += get_glyph(ch, font_size).advance;
					}
				}
				return off;
			}

			inline uvec2 text_size(uint font_size, std::wstring_view str)
			{
				auto size = uvec2(0, font_size);
				auto x = 0U;

				for (auto ch : str)
				{
					if (ch == '\n')
					{
						size.y += font_size;
						x = 0;
					}
					else if (ch != '\r')
					{
						if (ch == '\t')
							ch = ' ';
						x += get_glyph(ch, font_size).advance;
						size.x = max(size.x, x);
					}
				}
				return size;
			}

			inline std::wstring wrap_text(uint font_size, uint width, std::wstring_view str)
			{
				if (font_size > width)
				{
					assert(0);
					return L"";
				}

				auto ret = std::wstring();
				auto w = 0U;

				for (auto ch : str)
				{
					switch (ch)
					{
					case '\n':
						w = 0;
						ret += '\n';
						break;
					case '\r':
						continue;
					case '\t':
						ch = ' ';
					default:
						auto adv = get_glyph(ch, font_size).advance;
						if (w + adv >= width)
						{
							w = adv;
							ret += '\n';
						}
						else
							w += adv;
						ret += ch;
					}
				}

				return ret;
			}

			inline std::vector<GlyphDraw> get_draw_glyphs(uint size, std::wstring_view str, const vec2& pos = vec2(0.f), const mat2& axes = mat2(1.f))
			{
				std::vector<GlyphDraw> ret;
				ret.resize(str.size());

				auto i = 0;
				auto p = vec2(0.f);
				for (i = 0; i < str.size(); i++)
				{
					auto ch = str[i];
					if (ch == '\n')
					{
						p.y += size;
						p.x = 0.f;
					}
					else if (ch != '\r')
					{
						if (ch == '\t')
							ch = ' ';

						auto& g = get_glyph(ch, size);
						auto o = p + vec2(g.off);
						auto s = vec2(g.size);

						auto& dst = ret[i];
						dst.uvs = g.uv;
						dst.points[0] = pos + o * axes;
						dst.points[1] = pos + o.x * axes[0] + (o.y - s.y) * axes[1];
						dst.points[2] = pos + (o.x + s.x) * axes[0] + (o.y - s.y) * axes[1];
						dst.points[3] = pos + (o.x + s.x) * axes[0] + o.y * axes[1];

						p.x += g.advance;
					}
				}

				ret.resize(i);
				return ret;
			}

			struct Get
			{
				// font_names is seperated by ';'
				virtual FontAtlasPtr operator()(DevicePtr device, const std::wstring& font_names) = 0;
			};
			FLAME_GRAPHICS_EXPORTS static Get& get;
		};
	}

	// Font Awesome:

#define ICON_MIN_FA 0xe005
#define ICON_MAX_FA 0xf8ff
#define ICON_FA_AD L"\xef\x99\x81"	// U+f641
#define ICON_FA_ADDRESS_BOOK L"\xef\x8a\xb9"	// U+f2b9
#define ICON_FA_ADDRESS_CARD L"\xef\x8a\xbb"	// U+f2bb
#define ICON_FA_ADJUST L"\xef\x81\x82"	// U+f042
#define ICON_FA_AIR_FRESHENER L"\xef\x97\x90"	// U+f5d0
#define ICON_FA_ALIGN_CENTER L"\xef\x80\xb7"	// U+f037
#define ICON_FA_ALIGN_JUSTIFY L"\xef\x80\xb9"	// U+f039
#define ICON_FA_ALIGN_LEFT L"\xef\x80\xb6"	// U+f036
#define ICON_FA_ALIGN_RIGHT L"\xef\x80\xb8"	// U+f038
#define ICON_FA_ALLERGIES L"\xef\x91\xa1"	// U+f461
#define ICON_FA_AMBULANCE L"\xef\x83\xb9"	// U+f0f9
#define ICON_FA_AMERICAN_SIGN_LANGUAGE_INTERPRETING L"\xef\x8a\xa3"	// U+f2a3
#define ICON_FA_ANCHOR L"\xef\x84\xbd"	// U+f13d
#define ICON_FA_ANGLE_DOUBLE_DOWN L"\xef\x84\x83"	// U+f103
#define ICON_FA_ANGLE_DOUBLE_LEFT L"\xef\x84\x80"	// U+f100
#define ICON_FA_ANGLE_DOUBLE_RIGHT L"\xef\x84\x81"	// U+f101
#define ICON_FA_ANGLE_DOUBLE_UP L"\xef\x84\x82"	// U+f102
#define ICON_FA_ANGLE_DOWN L"\xef\x84\x87"	// U+f107
#define ICON_FA_ANGLE_LEFT L"\xef\x84\x84"	// U+f104
#define ICON_FA_ANGLE_RIGHT L"\xef\x84\x85"	// U+f105
#define ICON_FA_ANGLE_UP L"\xef\x84\x86"	// U+f106
#define ICON_FA_ANGRY L"\xef\x95\x96"	// U+f556
#define ICON_FA_ANKH L"\xef\x99\x84"	// U+f644
#define ICON_FA_APPLE_ALT L"\xef\x97\x91"	// U+f5d1
#define ICON_FA_ARCHIVE L"\xef\x86\x87"	// U+f187
#define ICON_FA_ARCHWAY L"\xef\x95\x97"	// U+f557
#define ICON_FA_ARROW_ALT_CIRCLE_DOWN L"\xef\x8d\x98"	// U+f358
#define ICON_FA_ARROW_ALT_CIRCLE_LEFT L"\xef\x8d\x99"	// U+f359
#define ICON_FA_ARROW_ALT_CIRCLE_RIGHT L"\xef\x8d\x9a"	// U+f35a
#define ICON_FA_ARROW_ALT_CIRCLE_UP L"\xef\x8d\x9b"	// U+f35b
#define ICON_FA_ARROW_CIRCLE_DOWN L"\xef\x82\xab"	// U+f0ab
#define ICON_FA_ARROW_CIRCLE_LEFT L"\xef\x82\xa8"	// U+f0a8
#define ICON_FA_ARROW_CIRCLE_RIGHT L"\xef\x82\xa9"	// U+f0a9
#define ICON_FA_ARROW_CIRCLE_UP L"\xef\x82\xaa"	// U+f0aa
#define ICON_FA_ARROW_DOWN L"\xef\x81\xa3"	// U+f063
#define ICON_FA_ARROW_LEFT L"\xef\x81\xa0"	// U+f060
#define ICON_FA_ARROW_RIGHT L"\xef\x81\xa1"	// U+f061
#define ICON_FA_ARROW_UP L"\xef\x81\xa2"	// U+f062
#define ICON_FA_ARROWS_ALT L"\xef\x82\xb2"	// U+f0b2
#define ICON_FA_ARROWS_ALT_H L"\xef\x8c\xb7"	// U+f337
#define ICON_FA_ARROWS_ALT_V L"\xef\x8c\xb8"	// U+f338
#define ICON_FA_ASSISTIVE_LISTENING_SYSTEMS L"\xef\x8a\xa2"	// U+f2a2
#define ICON_FA_ASTERISK L"\xef\x81\xa9"	// U+f069
#define ICON_FA_AT L"\xef\x87\xba"	// U+f1fa
#define ICON_FA_ATLAS L"\xef\x95\x98"	// U+f558
#define ICON_FA_ATOM L"\xef\x97\x92"	// U+f5d2
#define ICON_FA_AUDIO_DESCRIPTION L"\xef\x8a\x9e"	// U+f29e
#define ICON_FA_AWARD L"\xef\x95\x99"	// U+f559
#define ICON_FA_BABY L"\xef\x9d\xbc"	// U+f77c
#define ICON_FA_BABY_CARRIAGE L"\xef\x9d\xbd"	// U+f77d
#define ICON_FA_BACKSPACE L"\xef\x95\x9a"	// U+f55a
#define ICON_FA_BACKWARD L"\xef\x81\x8a"	// U+f04a
#define ICON_FA_BACON L"\xef\x9f\xa5"	// U+f7e5
#define ICON_FA_BACTERIA L"\xee\x81\x99"	// U+e059
#define ICON_FA_BACTERIUM L"\xee\x81\x9a"	// U+e05a
#define ICON_FA_BAHAI L"\xef\x99\xa6"	// U+f666
#define ICON_FA_BALANCE_SCALE L"\xef\x89\x8e"	// U+f24e
#define ICON_FA_BALANCE_SCALE_LEFT L"\xef\x94\x95"	// U+f515
#define ICON_FA_BALANCE_SCALE_RIGHT L"\xef\x94\x96"	// U+f516
#define ICON_FA_BAN L"\xef\x81\x9e"	// U+f05e
#define ICON_FA_BAND_AID L"\xef\x91\xa2"	// U+f462
#define ICON_FA_BARCODE L"\xef\x80\xaa"	// U+f02a
#define ICON_FA_BARS L"\xef\x83\x89"	// U+f0c9
#define ICON_FA_BASEBALL_BALL L"\xef\x90\xb3"	// U+f433
#define ICON_FA_BASKETBALL_BALL L"\xef\x90\xb4"	// U+f434
#define ICON_FA_BATH L"\xef\x8b\x8d"	// U+f2cd
#define ICON_FA_BATTERY_EMPTY L"\xef\x89\x84"	// U+f244
#define ICON_FA_BATTERY_FULL L"\xef\x89\x80"	// U+f240
#define ICON_FA_BATTERY_HALF L"\xef\x89\x82"	// U+f242
#define ICON_FA_BATTERY_QUARTER L"\xef\x89\x83"	// U+f243
#define ICON_FA_BATTERY_THREE_QUARTERS L"\xef\x89\x81"	// U+f241
#define ICON_FA_BED L"\xef\x88\xb6"	// U+f236
#define ICON_FA_BEER L"\xef\x83\xbc"	// U+f0fc
#define ICON_FA_BELL L"\xef\x83\xb3"	// U+f0f3
#define ICON_FA_BELL_SLASH L"\xef\x87\xb6"	// U+f1f6
#define ICON_FA_BEZIER_CURVE L"\xef\x95\x9b"	// U+f55b
#define ICON_FA_BIBLE L"\xef\x99\x87"	// U+f647
#define ICON_FA_BICYCLE L"\xef\x88\x86"	// U+f206
#define ICON_FA_BIKING L"\xef\xa1\x8a"	// U+f84a
#define ICON_FA_BINOCULARS L"\xef\x87\xa5"	// U+f1e5
#define ICON_FA_BIOHAZARD L"\xef\x9e\x80"	// U+f780
#define ICON_FA_BIRTHDAY_CAKE L"\xef\x87\xbd"	// U+f1fd
#define ICON_FA_BLENDER L"\xef\x94\x97"	// U+f517
#define ICON_FA_BLENDER_PHONE L"\xef\x9a\xb6"	// U+f6b6
#define ICON_FA_BLIND L"\xef\x8a\x9d"	// U+f29d
#define ICON_FA_BLOG L"\xef\x9e\x81"	// U+f781
#define ICON_FA_BOLD L"\xef\x80\xb2"	// U+f032
#define ICON_FA_BOLT L"\xef\x83\xa7"	// U+f0e7
#define ICON_FA_BOMB L"\xef\x87\xa2"	// U+f1e2
#define ICON_FA_BONE L"\xef\x97\x97"	// U+f5d7
#define ICON_FA_BONG L"\xef\x95\x9c"	// U+f55c
#define ICON_FA_BOOK L"\xef\x80\xad"	// U+f02d
#define ICON_FA_BOOK_DEAD L"\xef\x9a\xb7"	// U+f6b7
#define ICON_FA_BOOK_MEDICAL L"\xef\x9f\xa6"	// U+f7e6
#define ICON_FA_BOOK_OPEN L"\xef\x94\x98"	// U+f518
#define ICON_FA_BOOK_READER L"\xef\x97\x9a"	// U+f5da
#define ICON_FA_BOOKMARK L"\xef\x80\xae"	// U+f02e
#define ICON_FA_BORDER_ALL L"\xef\xa1\x8c"	// U+f84c
#define ICON_FA_BORDER_NONE L"\xef\xa1\x90"	// U+f850
#define ICON_FA_BORDER_STYLE L"\xef\xa1\x93"	// U+f853
#define ICON_FA_BOWLING_BALL L"\xef\x90\xb6"	// U+f436
#define ICON_FA_BOX L"\xef\x91\xa6"	// U+f466
#define ICON_FA_BOX_OPEN L"\xef\x92\x9e"	// U+f49e
#define ICON_FA_BOX_TISSUE L"\xee\x81\x9b"	// U+e05b
#define ICON_FA_BOXES L"\xef\x91\xa8"	// U+f468
#define ICON_FA_BRAILLE L"\xef\x8a\xa1"	// U+f2a1
#define ICON_FA_BRAIN L"\xef\x97\x9c"	// U+f5dc
#define ICON_FA_BREAD_SLICE L"\xef\x9f\xac"	// U+f7ec
#define ICON_FA_BRIEFCASE L"\xef\x82\xb1"	// U+f0b1
#define ICON_FA_BRIEFCASE_MEDICAL L"\xef\x91\xa9"	// U+f469
#define ICON_FA_BROADCAST_TOWER L"\xef\x94\x99"	// U+f519
#define ICON_FA_BROOM L"\xef\x94\x9a"	// U+f51a
#define ICON_FA_BRUSH L"\xef\x95\x9d"	// U+f55d
#define ICON_FA_BUG L"\xef\x86\x88"	// U+f188
#define ICON_FA_BUILDING L"\xef\x86\xad"	// U+f1ad
#define ICON_FA_BULLHORN L"\xef\x82\xa1"	// U+f0a1
#define ICON_FA_BULLSEYE L"\xef\x85\x80"	// U+f140
#define ICON_FA_BURN L"\xef\x91\xaa"	// U+f46a
#define ICON_FA_BUS L"\xef\x88\x87"	// U+f207
#define ICON_FA_BUS_ALT L"\xef\x95\x9e"	// U+f55e
#define ICON_FA_BUSINESS_TIME L"\xef\x99\x8a"	// U+f64a
#define ICON_FA_CALCULATOR L"\xef\x87\xac"	// U+f1ec
#define ICON_FA_CALENDAR L"\xef\x84\xb3"	// U+f133
#define ICON_FA_CALENDAR_ALT L"\xef\x81\xb3"	// U+f073
#define ICON_FA_CALENDAR_CHECK L"\xef\x89\xb4"	// U+f274
#define ICON_FA_CALENDAR_DAY L"\xef\x9e\x83"	// U+f783
#define ICON_FA_CALENDAR_MINUS L"\xef\x89\xb2"	// U+f272
#define ICON_FA_CALENDAR_PLUS L"\xef\x89\xb1"	// U+f271
#define ICON_FA_CALENDAR_TIMES L"\xef\x89\xb3"	// U+f273
#define ICON_FA_CALENDAR_WEEK L"\xef\x9e\x84"	// U+f784
#define ICON_FA_CAMERA L"\xef\x80\xb0"	// U+f030
#define ICON_FA_CAMERA_RETRO L"\xef\x82\x83"	// U+f083
#define ICON_FA_CAMPGROUND L"\xef\x9a\xbb"	// U+f6bb
#define ICON_FA_CANDY_CANE L"\xef\x9e\x86"	// U+f786
#define ICON_FA_CANNABIS L"\xef\x95\x9f"	// U+f55f
#define ICON_FA_CAPSULES L"\xef\x91\xab"	// U+f46b
#define ICON_FA_CAR L"\xef\x86\xb9"	// U+f1b9
#define ICON_FA_CAR_ALT L"\xef\x97\x9e"	// U+f5de
#define ICON_FA_CAR_BATTERY L"\xef\x97\x9f"	// U+f5df
#define ICON_FA_CAR_CRASH L"\xef\x97\xa1"	// U+f5e1
#define ICON_FA_CAR_SIDE L"\xef\x97\xa4"	// U+f5e4
#define ICON_FA_CARAVAN L"\xef\xa3\xbf"	// U+f8ff
#define ICON_FA_CARET_DOWN L"\xef\x83\x97"	// U+f0d7
#define ICON_FA_CARET_LEFT L"\xef\x83\x99"	// U+f0d9
#define ICON_FA_CARET_RIGHT L"\xef\x83\x9a"	// U+f0da
#define ICON_FA_CARET_SQUARE_DOWN L"\xef\x85\x90"	// U+f150
#define ICON_FA_CARET_SQUARE_LEFT L"\xef\x86\x91"	// U+f191
#define ICON_FA_CARET_SQUARE_RIGHT L"\xef\x85\x92"	// U+f152
#define ICON_FA_CARET_SQUARE_UP L"\xef\x85\x91"	// U+f151
#define ICON_FA_CARET_UP L"\xef\x83\x98"	// U+f0d8
#define ICON_FA_CARROT L"\xef\x9e\x87"	// U+f787
#define ICON_FA_CART_ARROW_DOWN L"\xef\x88\x98"	// U+f218
#define ICON_FA_CART_PLUS L"\xef\x88\x97"	// U+f217
#define ICON_FA_CASH_REGISTER L"\xef\x9e\x88"	// U+f788
#define ICON_FA_CAT L"\xef\x9a\xbe"	// U+f6be
#define ICON_FA_CERTIFICATE L"\xef\x82\xa3"	// U+f0a3
#define ICON_FA_CHAIR L"\xef\x9b\x80"	// U+f6c0
#define ICON_FA_CHALKBOARD L"\xef\x94\x9b"	// U+f51b
#define ICON_FA_CHALKBOARD_TEACHER L"\xef\x94\x9c"	// U+f51c
#define ICON_FA_CHARGING_STATION L"\xef\x97\xa7"	// U+f5e7
#define ICON_FA_CHART_AREA L"\xef\x87\xbe"	// U+f1fe
#define ICON_FA_CHART_BAR L"\xef\x82\x80"	// U+f080
#define ICON_FA_CHART_LINE L"\xef\x88\x81"	// U+f201
#define ICON_FA_CHART_PIE L"\xef\x88\x80"	// U+f200
#define ICON_FA_CHECK L"\xef\x80\x8c"	// U+f00c
#define ICON_FA_CHECK_CIRCLE L"\xef\x81\x98"	// U+f058
#define ICON_FA_CHECK_DOUBLE L"\xef\x95\xa0"	// U+f560
#define ICON_FA_CHECK_SQUARE L"\xef\x85\x8a"	// U+f14a
#define ICON_FA_CHEESE L"\xef\x9f\xaf"	// U+f7ef
#define ICON_FA_CHESS L"\xef\x90\xb9"	// U+f439
#define ICON_FA_CHESS_BISHOP L"\xef\x90\xba"	// U+f43a
#define ICON_FA_CHESS_BOARD L"\xef\x90\xbc"	// U+f43c
#define ICON_FA_CHESS_KING L"\xef\x90\xbf"	// U+f43f
#define ICON_FA_CHESS_KNIGHT L"\xef\x91\x81"	// U+f441
#define ICON_FA_CHESS_PAWN L"\xef\x91\x83"	// U+f443
#define ICON_FA_CHESS_QUEEN L"\xef\x91\x85"	// U+f445
#define ICON_FA_CHESS_ROOK L"\xef\x91\x87"	// U+f447
#define ICON_FA_CHEVRON_CIRCLE_DOWN L"\xef\x84\xba"	// U+f13a
#define ICON_FA_CHEVRON_CIRCLE_LEFT L"\xef\x84\xb7"	// U+f137
#define ICON_FA_CHEVRON_CIRCLE_RIGHT L"\xef\x84\xb8"	// U+f138
#define ICON_FA_CHEVRON_CIRCLE_UP L"\xef\x84\xb9"	// U+f139
#define ICON_FA_CHEVRON_DOWN L"\xef\x81\xb8"	// U+f078
#define ICON_FA_CHEVRON_LEFT L"\xef\x81\x93"	// U+f053
#define ICON_FA_CHEVRON_RIGHT L"\xef\x81\x94"	// U+f054
#define ICON_FA_CHEVRON_UP L"\xef\x81\xb7"	// U+f077
#define ICON_FA_CHILD L"\xef\x86\xae"	// U+f1ae
#define ICON_FA_CHURCH L"\xef\x94\x9d"	// U+f51d
#define ICON_FA_CIRCLE L"\xef\x84\x91"	// U+f111
#define ICON_FA_CIRCLE_NOTCH L"\xef\x87\x8e"	// U+f1ce
#define ICON_FA_CITY L"\xef\x99\x8f"	// U+f64f
#define ICON_FA_CLINIC_MEDICAL L"\xef\x9f\xb2"	// U+f7f2
#define ICON_FA_CLIPBOARD L"\xef\x8c\xa8"	// U+f328
#define ICON_FA_CLIPBOARD_CHECK L"\xef\x91\xac"	// U+f46c
#define ICON_FA_CLIPBOARD_LIST L"\xef\x91\xad"	// U+f46d
#define ICON_FA_CLOCK L"\xef\x80\x97"	// U+f017
#define ICON_FA_CLONE L"\xef\x89\x8d"	// U+f24d
#define ICON_FA_CLOSED_CAPTIONING L"\xef\x88\x8a"	// U+f20a
#define ICON_FA_CLOUD L"\xef\x83\x82"	// U+f0c2
#define ICON_FA_CLOUD_DOWNLOAD_ALT L"\xef\x8e\x81"	// U+f381
#define ICON_FA_CLOUD_MEATBALL L"\xef\x9c\xbb"	// U+f73b
#define ICON_FA_CLOUD_MOON L"\xef\x9b\x83"	// U+f6c3
#define ICON_FA_CLOUD_MOON_RAIN L"\xef\x9c\xbc"	// U+f73c
#define ICON_FA_CLOUD_RAIN L"\xef\x9c\xbd"	// U+f73d
#define ICON_FA_CLOUD_SHOWERS_HEAVY L"\xef\x9d\x80"	// U+f740
#define ICON_FA_CLOUD_SUN L"\xef\x9b\x84"	// U+f6c4
#define ICON_FA_CLOUD_SUN_RAIN L"\xef\x9d\x83"	// U+f743
#define ICON_FA_CLOUD_UPLOAD_ALT L"\xef\x8e\x82"	// U+f382
#define ICON_FA_COCKTAIL L"\xef\x95\xa1"	// U+f561
#define ICON_FA_CODE L"\xef\x84\xa1"	// U+f121
#define ICON_FA_CODE_BRANCH L"\xef\x84\xa6"	// U+f126
#define ICON_FA_COFFEE L"\xef\x83\xb4"	// U+f0f4
#define ICON_FA_COG L"\xef\x80\x93"	// U+f013
#define ICON_FA_COGS L"\xef\x82\x85"	// U+f085
#define ICON_FA_COINS L"\xef\x94\x9e"	// U+f51e
#define ICON_FA_COLUMNS L"\xef\x83\x9b"	// U+f0db
#define ICON_FA_COMMENT L"\xef\x81\xb5"	// U+f075
#define ICON_FA_COMMENT_ALT L"\xef\x89\xba"	// U+f27a
#define ICON_FA_COMMENT_DOLLAR L"\xef\x99\x91"	// U+f651
#define ICON_FA_COMMENT_DOTS L"\xef\x92\xad"	// U+f4ad
#define ICON_FA_COMMENT_MEDICAL L"\xef\x9f\xb5"	// U+f7f5
#define ICON_FA_COMMENT_SLASH L"\xef\x92\xb3"	// U+f4b3
#define ICON_FA_COMMENTS L"\xef\x82\x86"	// U+f086
#define ICON_FA_COMMENTS_DOLLAR L"\xef\x99\x93"	// U+f653
#define ICON_FA_COMPACT_DISC L"\xef\x94\x9f"	// U+f51f
#define ICON_FA_COMPASS L"\xef\x85\x8e"	// U+f14e
#define ICON_FA_COMPRESS L"\xef\x81\xa6"	// U+f066
#define ICON_FA_COMPRESS_ALT L"\xef\x90\xa2"	// U+f422
#define ICON_FA_COMPRESS_ARROWS_ALT L"\xef\x9e\x8c"	// U+f78c
#define ICON_FA_CONCIERGE_BELL L"\xef\x95\xa2"	// U+f562
#define ICON_FA_COOKIE L"\xef\x95\xa3"	// U+f563
#define ICON_FA_COOKIE_BITE L"\xef\x95\xa4"	// U+f564
#define ICON_FA_COPY L"\xef\x83\x85"	// U+f0c5
#define ICON_FA_COPYRIGHT L"\xef\x87\xb9"	// U+f1f9
#define ICON_FA_COUCH L"\xef\x92\xb8"	// U+f4b8
#define ICON_FA_CREDIT_CARD L"\xef\x82\x9d"	// U+f09d
#define ICON_FA_CROP L"\xef\x84\xa5"	// U+f125
#define ICON_FA_CROP_ALT L"\xef\x95\xa5"	// U+f565
#define ICON_FA_CROSS L"\xef\x99\x94"	// U+f654
#define ICON_FA_CROSSHAIRS L"\xef\x81\x9b"	// U+f05b
#define ICON_FA_CROW L"\xef\x94\xa0"	// U+f520
#define ICON_FA_CROWN L"\xef\x94\xa1"	// U+f521
#define ICON_FA_CRUTCH L"\xef\x9f\xb7"	// U+f7f7
#define ICON_FA_CUBE L"\xef\x86\xb2"	// U+f1b2
#define ICON_FA_CUBES L"\xef\x86\xb3"	// U+f1b3
#define ICON_FA_CUT L"\xef\x83\x84"	// U+f0c4
#define ICON_FA_DATABASE L"\xef\x87\x80"	// U+f1c0
#define ICON_FA_DEAF L"\xef\x8a\xa4"	// U+f2a4
#define ICON_FA_DEMOCRAT L"\xef\x9d\x87"	// U+f747
#define ICON_FA_DESKTOP L"\xef\x84\x88"	// U+f108
#define ICON_FA_DHARMACHAKRA L"\xef\x99\x95"	// U+f655
#define ICON_FA_DIAGNOSES L"\xef\x91\xb0"	// U+f470
#define ICON_FA_DICE L"\xef\x94\xa2"	// U+f522
#define ICON_FA_DICE_D20 L"\xef\x9b\x8f"	// U+f6cf
#define ICON_FA_DICE_D6 L"\xef\x9b\x91"	// U+f6d1
#define ICON_FA_DICE_FIVE L"\xef\x94\xa3"	// U+f523
#define ICON_FA_DICE_FOUR L"\xef\x94\xa4"	// U+f524
#define ICON_FA_DICE_ONE L"\xef\x94\xa5"	// U+f525
#define ICON_FA_DICE_SIX L"\xef\x94\xa6"	// U+f526
#define ICON_FA_DICE_THREE L"\xef\x94\xa7"	// U+f527
#define ICON_FA_DICE_TWO L"\xef\x94\xa8"	// U+f528
#define ICON_FA_DIGITAL_TACHOGRAPH L"\xef\x95\xa6"	// U+f566
#define ICON_FA_DIRECTIONS L"\xef\x97\xab"	// U+f5eb
#define ICON_FA_DISEASE L"\xef\x9f\xba"	// U+f7fa
#define ICON_FA_DIVIDE L"\xef\x94\xa9"	// U+f529
#define ICON_FA_DIZZY L"\xef\x95\xa7"	// U+f567
#define ICON_FA_DNA L"\xef\x91\xb1"	// U+f471
#define ICON_FA_DOG L"\xef\x9b\x93"	// U+f6d3
#define ICON_FA_DOLLAR_SIGN L"\xef\x85\x95"	// U+f155
#define ICON_FA_DOLLY L"\xef\x91\xb2"	// U+f472
#define ICON_FA_DOLLY_FLATBED L"\xef\x91\xb4"	// U+f474
#define ICON_FA_DONATE L"\xef\x92\xb9"	// U+f4b9
#define ICON_FA_DOOR_CLOSED L"\xef\x94\xaa"	// U+f52a
#define ICON_FA_DOOR_OPEN L"\xef\x94\xab"	// U+f52b
#define ICON_FA_DOT_CIRCLE L"\xef\x86\x92"	// U+f192
#define ICON_FA_DOVE L"\xef\x92\xba"	// U+f4ba
#define ICON_FA_DOWNLOAD L"\xef\x80\x99"	// U+f019
#define ICON_FA_DRAFTING_COMPASS L"\xef\x95\xa8"	// U+f568
#define ICON_FA_DRAGON L"\xef\x9b\x95"	// U+f6d5
#define ICON_FA_DRAW_POLYGON L"\xef\x97\xae"	// U+f5ee
#define ICON_FA_DRUM L"\xef\x95\xa9"	// U+f569
#define ICON_FA_DRUM_STEELPAN L"\xef\x95\xaa"	// U+f56a
#define ICON_FA_DRUMSTICK_BITE L"\xef\x9b\x97"	// U+f6d7
#define ICON_FA_DUMBBELL L"\xef\x91\x8b"	// U+f44b
#define ICON_FA_DUMPSTER L"\xef\x9e\x93"	// U+f793
#define ICON_FA_DUMPSTER_FIRE L"\xef\x9e\x94"	// U+f794
#define ICON_FA_DUNGEON L"\xef\x9b\x99"	// U+f6d9
#define ICON_FA_EDIT L"\xef\x81\x84"	// U+f044
#define ICON_FA_EGG L"\xef\x9f\xbb"	// U+f7fb
#define ICON_FA_EJECT L"\xef\x81\x92"	// U+f052
#define ICON_FA_ELLIPSIS_H L"\xef\x85\x81"	// U+f141
#define ICON_FA_ELLIPSIS_V L"\xef\x85\x82"	// U+f142
#define ICON_FA_ENVELOPE L"\xef\x83\xa0"	// U+f0e0
#define ICON_FA_ENVELOPE_OPEN L"\xef\x8a\xb6"	// U+f2b6
#define ICON_FA_ENVELOPE_OPEN_TEXT L"\xef\x99\x98"	// U+f658
#define ICON_FA_ENVELOPE_SQUARE L"\xef\x86\x99"	// U+f199
#define ICON_FA_EQUALS L"\xef\x94\xac"	// U+f52c
#define ICON_FA_ERASER L"\xef\x84\xad"	// U+f12d
#define ICON_FA_ETHERNET L"\xef\x9e\x96"	// U+f796
#define ICON_FA_EURO_SIGN L"\xef\x85\x93"	// U+f153
#define ICON_FA_EXCHANGE_ALT L"\xef\x8d\xa2"	// U+f362
#define ICON_FA_EXCLAMATION L"\xef\x84\xaa"	// U+f12a
#define ICON_FA_EXCLAMATION_CIRCLE L"\xef\x81\xaa"	// U+f06a
#define ICON_FA_EXCLAMATION_TRIANGLE L"\xef\x81\xb1"	// U+f071
#define ICON_FA_EXPAND L"\xef\x81\xa5"	// U+f065
#define ICON_FA_EXPAND_ALT L"\xef\x90\xa4"	// U+f424
#define ICON_FA_EXPAND_ARROWS_ALT L"\xef\x8c\x9e"	// U+f31e
#define ICON_FA_EXTERNAL_LINK_ALT L"\xef\x8d\x9d"	// U+f35d
#define ICON_FA_EXTERNAL_LINK_SQUARE_ALT L"\xef\x8d\xa0"	// U+f360
#define ICON_FA_EYE L"\xef\x81\xae"	// U+f06e
#define ICON_FA_EYE_DROPPER L"\xef\x87\xbb"	// U+f1fb
#define ICON_FA_EYE_SLASH L"\xef\x81\xb0"	// U+f070
#define ICON_FA_FAN L"\xef\xa1\xa3"	// U+f863
#define ICON_FA_FAST_BACKWARD L"\xef\x81\x89"	// U+f049
#define ICON_FA_FAST_FORWARD L"\xef\x81\x90"	// U+f050
#define ICON_FA_FAUCET L"\xee\x80\x85"	// U+e005
#define ICON_FA_FAX L"\xef\x86\xac"	// U+f1ac
#define ICON_FA_FEATHER L"\xef\x94\xad"	// U+f52d
#define ICON_FA_FEATHER_ALT L"\xef\x95\xab"	// U+f56b
#define ICON_FA_FEMALE L"\xef\x86\x82"	// U+f182
#define ICON_FA_FIGHTER_JET L"\xef\x83\xbb"	// U+f0fb
#define ICON_FA_FILE L"\xef\x85\x9b"	// U+f15b
#define ICON_FA_FILE_ALT L"\xef\x85\x9c"	// U+f15c
#define ICON_FA_FILE_ARCHIVE L"\xef\x87\x86"	// U+f1c6
#define ICON_FA_FILE_AUDIO L"\xef\x87\x87"	// U+f1c7
#define ICON_FA_FILE_CODE L"\xef\x87\x89"	// U+f1c9
#define ICON_FA_FILE_CONTRACT L"\xef\x95\xac"	// U+f56c
#define ICON_FA_FILE_CSV L"\xef\x9b\x9d"	// U+f6dd
#define ICON_FA_FILE_DOWNLOAD L"\xef\x95\xad"	// U+f56d
#define ICON_FA_FILE_EXCEL L"\xef\x87\x83"	// U+f1c3
#define ICON_FA_FILE_EXPORT L"\xef\x95\xae"	// U+f56e
#define ICON_FA_FILE_IMAGE L"\xef\x87\x85"	// U+f1c5
#define ICON_FA_FILE_IMPORT L"\xef\x95\xaf"	// U+f56f
#define ICON_FA_FILE_INVOICE L"\xef\x95\xb0"	// U+f570
#define ICON_FA_FILE_INVOICE_DOLLAR L"\xef\x95\xb1"	// U+f571
#define ICON_FA_FILE_MEDICAL L"\xef\x91\xb7"	// U+f477
#define ICON_FA_FILE_MEDICAL_ALT L"\xef\x91\xb8"	// U+f478
#define ICON_FA_FILE_PDF L"\xef\x87\x81"	// U+f1c1
#define ICON_FA_FILE_POWERPOINT L"\xef\x87\x84"	// U+f1c4
#define ICON_FA_FILE_PRESCRIPTION L"\xef\x95\xb2"	// U+f572
#define ICON_FA_FILE_SIGNATURE L"\xef\x95\xb3"	// U+f573
#define ICON_FA_FILE_UPLOAD L"\xef\x95\xb4"	// U+f574
#define ICON_FA_FILE_VIDEO L"\xef\x87\x88"	// U+f1c8
#define ICON_FA_FILE_WORD L"\xef\x87\x82"	// U+f1c2
#define ICON_FA_FILL L"\xef\x95\xb5"	// U+f575
#define ICON_FA_FILL_DRIP L"\xef\x95\xb6"	// U+f576
#define ICON_FA_FILM L"\xef\x80\x88"	// U+f008
#define ICON_FA_FILTER L"\xef\x82\xb0"	// U+f0b0
#define ICON_FA_FINGERPRINT L"\xef\x95\xb7"	// U+f577
#define ICON_FA_FIRE L"\xef\x81\xad"	// U+f06d
#define ICON_FA_FIRE_ALT L"\xef\x9f\xa4"	// U+f7e4
#define ICON_FA_FIRE_EXTINGUISHER L"\xef\x84\xb4"	// U+f134
#define ICON_FA_FIRST_AID L"\xef\x91\xb9"	// U+f479
#define ICON_FA_FISH L"\xef\x95\xb8"	// U+f578
#define ICON_FA_FIST_RAISED L"\xef\x9b\x9e"	// U+f6de
#define ICON_FA_FLAG L"\xef\x80\xa4"	// U+f024
#define ICON_FA_FLAG_CHECKERED L"\xef\x84\x9e"	// U+f11e
#define ICON_FA_FLAG_USA L"\xef\x9d\x8d"	// U+f74d
#define ICON_FA_FLASK L"\xef\x83\x83"	// U+f0c3
#define ICON_FA_FLUSHED L"\xef\x95\xb9"	// U+f579
#define ICON_FA_FOLDER L"\xef\x81\xbb"	// U+f07b
#define ICON_FA_FOLDER_MINUS L"\xef\x99\x9d"	// U+f65d
#define ICON_FA_FOLDER_OPEN L"\xef\x81\xbc"	// U+f07c
#define ICON_FA_FOLDER_PLUS L"\xef\x99\x9e"	// U+f65e
#define ICON_FA_FONT L"\xef\x80\xb1"	// U+f031
#define ICON_FA_FONT_AWESOME_LOGO_FULL L"\xef\x93\xa6"	// U+f4e6
#define ICON_FA_FOOTBALL_BALL L"\xef\x91\x8e"	// U+f44e
#define ICON_FA_FORWARD L"\xef\x81\x8e"	// U+f04e
#define ICON_FA_FROG L"\xef\x94\xae"	// U+f52e
#define ICON_FA_FROWN L"\xef\x84\x99"	// U+f119
#define ICON_FA_FROWN_OPEN L"\xef\x95\xba"	// U+f57a
#define ICON_FA_FUNNEL_DOLLAR L"\xef\x99\xa2"	// U+f662
#define ICON_FA_FUTBOL L"\xef\x87\xa3"	// U+f1e3
#define ICON_FA_GAMEPAD L"\xef\x84\x9b"	// U+f11b
#define ICON_FA_GAS_PUMP L"\xef\x94\xaf"	// U+f52f
#define ICON_FA_GAVEL L"\xef\x83\xa3"	// U+f0e3
#define ICON_FA_GEM L"\xef\x8e\xa5"	// U+f3a5
#define ICON_FA_GENDERLESS L"\xef\x88\xad"	// U+f22d
#define ICON_FA_GHOST L"\xef\x9b\xa2"	// U+f6e2
#define ICON_FA_GIFT L"\xef\x81\xab"	// U+f06b
#define ICON_FA_GIFTS L"\xef\x9e\x9c"	// U+f79c
#define ICON_FA_GLASS_CHEERS L"\xef\x9e\x9f"	// U+f79f
#define ICON_FA_GLASS_MARTINI L"\xef\x80\x80"	// U+f000
#define ICON_FA_GLASS_MARTINI_ALT L"\xef\x95\xbb"	// U+f57b
#define ICON_FA_GLASS_WHISKEY L"\xef\x9e\xa0"	// U+f7a0
#define ICON_FA_GLASSES L"\xef\x94\xb0"	// U+f530
#define ICON_FA_GLOBE L"\xef\x82\xac"	// U+f0ac
#define ICON_FA_GLOBE_AFRICA L"\xef\x95\xbc"	// U+f57c
#define ICON_FA_GLOBE_AMERICAS L"\xef\x95\xbd"	// U+f57d
#define ICON_FA_GLOBE_ASIA L"\xef\x95\xbe"	// U+f57e
#define ICON_FA_GLOBE_EUROPE L"\xef\x9e\xa2"	// U+f7a2
#define ICON_FA_GOLF_BALL L"\xef\x91\x90"	// U+f450
#define ICON_FA_GOPURAM L"\xef\x99\xa4"	// U+f664
#define ICON_FA_GRADUATION_CAP L"\xef\x86\x9d"	// U+f19d
#define ICON_FA_GREATER_THAN L"\xef\x94\xb1"	// U+f531
#define ICON_FA_GREATER_THAN_EQUAL L"\xef\x94\xb2"	// U+f532
#define ICON_FA_GRIMACE L"\xef\x95\xbf"	// U+f57f
#define ICON_FA_GRIN L"\xef\x96\x80"	// U+f580
#define ICON_FA_GRIN_ALT L"\xef\x96\x81"	// U+f581
#define ICON_FA_GRIN_BEAM L"\xef\x96\x82"	// U+f582
#define ICON_FA_GRIN_BEAM_SWEAT L"\xef\x96\x83"	// U+f583
#define ICON_FA_GRIN_HEARTS L"\xef\x96\x84"	// U+f584
#define ICON_FA_GRIN_SQUINT L"\xef\x96\x85"	// U+f585
#define ICON_FA_GRIN_SQUINT_TEARS L"\xef\x96\x86"	// U+f586
#define ICON_FA_GRIN_STARS L"\xef\x96\x87"	// U+f587
#define ICON_FA_GRIN_TEARS L"\xef\x96\x88"	// U+f588
#define ICON_FA_GRIN_TONGUE L"\xef\x96\x89"	// U+f589
#define ICON_FA_GRIN_TONGUE_SQUINT L"\xef\x96\x8a"	// U+f58a
#define ICON_FA_GRIN_TONGUE_WINK L"\xef\x96\x8b"	// U+f58b
#define ICON_FA_GRIN_WINK L"\xef\x96\x8c"	// U+f58c
#define ICON_FA_GRIP_HORIZONTAL L"\xef\x96\x8d"	// U+f58d
#define ICON_FA_GRIP_LINES L"\xef\x9e\xa4"	// U+f7a4
#define ICON_FA_GRIP_LINES_VERTICAL L"\xef\x9e\xa5"	// U+f7a5
#define ICON_FA_GRIP_VERTICAL L"\xef\x96\x8e"	// U+f58e
#define ICON_FA_GUITAR L"\xef\x9e\xa6"	// U+f7a6
#define ICON_FA_H_SQUARE L"\xef\x83\xbd"	// U+f0fd
#define ICON_FA_HAMBURGER L"\xef\xa0\x85"	// U+f805
#define ICON_FA_HAMMER L"\xef\x9b\xa3"	// U+f6e3
#define ICON_FA_HAMSA L"\xef\x99\xa5"	// U+f665
#define ICON_FA_HAND_HOLDING L"\xef\x92\xbd"	// U+f4bd
#define ICON_FA_HAND_HOLDING_HEART L"\xef\x92\xbe"	// U+f4be
#define ICON_FA_HAND_HOLDING_MEDICAL L"\xee\x81\x9c"	// U+e05c
#define ICON_FA_HAND_HOLDING_USD L"\xef\x93\x80"	// U+f4c0
#define ICON_FA_HAND_HOLDING_WATER L"\xef\x93\x81"	// U+f4c1
#define ICON_FA_HAND_LIZARD L"\xef\x89\x98"	// U+f258
#define ICON_FA_HAND_MIDDLE_FINGER L"\xef\xa0\x86"	// U+f806
#define ICON_FA_HAND_PAPER L"\xef\x89\x96"	// U+f256
#define ICON_FA_HAND_PEACE L"\xef\x89\x9b"	// U+f25b
#define ICON_FA_HAND_POINT_DOWN L"\xef\x82\xa7"	// U+f0a7
#define ICON_FA_HAND_POINT_LEFT L"\xef\x82\xa5"	// U+f0a5
#define ICON_FA_HAND_POINT_RIGHT L"\xef\x82\xa4"	// U+f0a4
#define ICON_FA_HAND_POINT_UP L"\xef\x82\xa6"	// U+f0a6
#define ICON_FA_HAND_POINTER L"\xef\x89\x9a"	// U+f25a
#define ICON_FA_HAND_ROCK L"\xef\x89\x95"	// U+f255
#define ICON_FA_HAND_SCISSORS L"\xef\x89\x97"	// U+f257
#define ICON_FA_HAND_SPARKLES L"\xee\x81\x9d"	// U+e05d
#define ICON_FA_HAND_SPOCK L"\xef\x89\x99"	// U+f259
#define ICON_FA_HANDS L"\xef\x93\x82"	// U+f4c2
#define ICON_FA_HANDS_HELPING L"\xef\x93\x84"	// U+f4c4
#define ICON_FA_HANDS_WASH L"\xee\x81\x9e"	// U+e05e
#define ICON_FA_HANDSHAKE L"\xef\x8a\xb5"	// U+f2b5
#define ICON_FA_HANDSHAKE_ALT_SLASH L"\xee\x81\x9f"	// U+e05f
#define ICON_FA_HANDSHAKE_SLASH L"\xee\x81\xa0"	// U+e060
#define ICON_FA_HANUKIAH L"\xef\x9b\xa6"	// U+f6e6
#define ICON_FA_HARD_HAT L"\xef\xa0\x87"	// U+f807
#define ICON_FA_HASHTAG L"\xef\x8a\x92"	// U+f292
#define ICON_FA_HAT_COWBOY L"\xef\xa3\x80"	// U+f8c0
#define ICON_FA_HAT_COWBOY_SIDE L"\xef\xa3\x81"	// U+f8c1
#define ICON_FA_HAT_WIZARD L"\xef\x9b\xa8"	// U+f6e8
#define ICON_FA_HDD L"\xef\x82\xa0"	// U+f0a0
#define ICON_FA_HEAD_SIDE_COUGH L"\xee\x81\xa1"	// U+e061
#define ICON_FA_HEAD_SIDE_COUGH_SLASH L"\xee\x81\xa2"	// U+e062
#define ICON_FA_HEAD_SIDE_MASK L"\xee\x81\xa3"	// U+e063
#define ICON_FA_HEAD_SIDE_VIRUS L"\xee\x81\xa4"	// U+e064
#define ICON_FA_HEADING L"\xef\x87\x9c"	// U+f1dc
#define ICON_FA_HEADPHONES L"\xef\x80\xa5"	// U+f025
#define ICON_FA_HEADPHONES_ALT L"\xef\x96\x8f"	// U+f58f
#define ICON_FA_HEADSET L"\xef\x96\x90"	// U+f590
#define ICON_FA_HEART L"\xef\x80\x84"	// U+f004
#define ICON_FA_HEART_BROKEN L"\xef\x9e\xa9"	// U+f7a9
#define ICON_FA_HEARTBEAT L"\xef\x88\x9e"	// U+f21e
#define ICON_FA_HELICOPTER L"\xef\x94\xb3"	// U+f533
#define ICON_FA_HIGHLIGHTER L"\xef\x96\x91"	// U+f591
#define ICON_FA_HIKING L"\xef\x9b\xac"	// U+f6ec
#define ICON_FA_HIPPO L"\xef\x9b\xad"	// U+f6ed
#define ICON_FA_HISTORY L"\xef\x87\x9a"	// U+f1da
#define ICON_FA_HOCKEY_PUCK L"\xef\x91\x93"	// U+f453
#define ICON_FA_HOLLY_BERRY L"\xef\x9e\xaa"	// U+f7aa
#define ICON_FA_HOME L"\xef\x80\x95"	// U+f015
#define ICON_FA_HORSE L"\xef\x9b\xb0"	// U+f6f0
#define ICON_FA_HORSE_HEAD L"\xef\x9e\xab"	// U+f7ab
#define ICON_FA_HOSPITAL L"\xef\x83\xb8"	// U+f0f8
#define ICON_FA_HOSPITAL_ALT L"\xef\x91\xbd"	// U+f47d
#define ICON_FA_HOSPITAL_SYMBOL L"\xef\x91\xbe"	// U+f47e
#define ICON_FA_HOSPITAL_USER L"\xef\xa0\x8d"	// U+f80d
#define ICON_FA_HOT_TUB L"\xef\x96\x93"	// U+f593
#define ICON_FA_HOTDOG L"\xef\xa0\x8f"	// U+f80f
#define ICON_FA_HOTEL L"\xef\x96\x94"	// U+f594
#define ICON_FA_HOURGLASS L"\xef\x89\x94"	// U+f254
#define ICON_FA_HOURGLASS_END L"\xef\x89\x93"	// U+f253
#define ICON_FA_HOURGLASS_HALF L"\xef\x89\x92"	// U+f252
#define ICON_FA_HOURGLASS_START L"\xef\x89\x91"	// U+f251
#define ICON_FA_HOUSE_DAMAGE L"\xef\x9b\xb1"	// U+f6f1
#define ICON_FA_HOUSE_USER L"\xee\x81\xa5"	// U+e065
#define ICON_FA_HRYVNIA L"\xef\x9b\xb2"	// U+f6f2
#define ICON_FA_I_CURSOR L"\xef\x89\x86"	// U+f246
#define ICON_FA_ICE_CREAM L"\xef\xa0\x90"	// U+f810
#define ICON_FA_ICICLES L"\xef\x9e\xad"	// U+f7ad
#define ICON_FA_ICONS L"\xef\xa1\xad"	// U+f86d
#define ICON_FA_ID_BADGE L"\xef\x8b\x81"	// U+f2c1
#define ICON_FA_ID_CARD L"\xef\x8b\x82"	// U+f2c2
#define ICON_FA_ID_CARD_ALT L"\xef\x91\xbf"	// U+f47f
#define ICON_FA_IGLOO L"\xef\x9e\xae"	// U+f7ae
#define ICON_FA_IMAGE L"\xef\x80\xbe"	// U+f03e
#define ICON_FA_IMAGES L"\xef\x8c\x82"	// U+f302
#define ICON_FA_INBOX L"\xef\x80\x9c"	// U+f01c
#define ICON_FA_INDENT L"\xef\x80\xbc"	// U+f03c
#define ICON_FA_INDUSTRY L"\xef\x89\xb5"	// U+f275
#define ICON_FA_INFINITY L"\xef\x94\xb4"	// U+f534
#define ICON_FA_INFO L"\xef\x84\xa9"	// U+f129
#define ICON_FA_INFO_CIRCLE L"\xef\x81\x9a"	// U+f05a
#define ICON_FA_ITALIC L"\xef\x80\xb3"	// U+f033
#define ICON_FA_JEDI L"\xef\x99\xa9"	// U+f669
#define ICON_FA_JOINT L"\xef\x96\x95"	// U+f595
#define ICON_FA_JOURNAL_WHILLS L"\xef\x99\xaa"	// U+f66a
#define ICON_FA_KAABA L"\xef\x99\xab"	// U+f66b
#define ICON_FA_KEY L"\xef\x82\x84"	// U+f084
#define ICON_FA_KEYBOARD L"\xef\x84\x9c"	// U+f11c
#define ICON_FA_KHANDA L"\xef\x99\xad"	// U+f66d
#define ICON_FA_KISS L"\xef\x96\x96"	// U+f596
#define ICON_FA_KISS_BEAM L"\xef\x96\x97"	// U+f597
#define ICON_FA_KISS_WINK_HEART L"\xef\x96\x98"	// U+f598
#define ICON_FA_KIWI_BIRD L"\xef\x94\xb5"	// U+f535
#define ICON_FA_LANDMARK L"\xef\x99\xaf"	// U+f66f
#define ICON_FA_LANGUAGE L"\xef\x86\xab"	// U+f1ab
#define ICON_FA_LAPTOP L"\xef\x84\x89"	// U+f109
#define ICON_FA_LAPTOP_CODE L"\xef\x97\xbc"	// U+f5fc
#define ICON_FA_LAPTOP_HOUSE L"\xee\x81\xa6"	// U+e066
#define ICON_FA_LAPTOP_MEDICAL L"\xef\xa0\x92"	// U+f812
#define ICON_FA_LAUGH L"\xef\x96\x99"	// U+f599
#define ICON_FA_LAUGH_BEAM L"\xef\x96\x9a"	// U+f59a
#define ICON_FA_LAUGH_SQUINT L"\xef\x96\x9b"	// U+f59b
#define ICON_FA_LAUGH_WINK L"\xef\x96\x9c"	// U+f59c
#define ICON_FA_LAYER_GROUP L"\xef\x97\xbd"	// U+f5fd
#define ICON_FA_LEAF L"\xef\x81\xac"	// U+f06c
#define ICON_FA_LEMON L"\xef\x82\x94"	// U+f094
#define ICON_FA_LESS_THAN L"\xef\x94\xb6"	// U+f536
#define ICON_FA_LESS_THAN_EQUAL L"\xef\x94\xb7"	// U+f537
#define ICON_FA_LEVEL_DOWN_ALT L"\xef\x8e\xbe"	// U+f3be
#define ICON_FA_LEVEL_UP_ALT L"\xef\x8e\xbf"	// U+f3bf
#define ICON_FA_LIFE_RING L"\xef\x87\x8d"	// U+f1cd
#define ICON_FA_LIGHTBULB L"\xef\x83\xab"	// U+f0eb
#define ICON_FA_LINK L"\xef\x83\x81"	// U+f0c1
#define ICON_FA_LIRA_SIGN L"\xef\x86\x95"	// U+f195
#define ICON_FA_LIST L"\xef\x80\xba"	// U+f03a
#define ICON_FA_LIST_ALT L"\xef\x80\xa2"	// U+f022
#define ICON_FA_LIST_OL L"\xef\x83\x8b"	// U+f0cb
#define ICON_FA_LIST_UL L"\xef\x83\x8a"	// U+f0ca
#define ICON_FA_LOCATION_ARROW L"\xef\x84\xa4"	// U+f124
#define ICON_FA_LOCK L"\xef\x80\xa3"	// U+f023
#define ICON_FA_LOCK_OPEN L"\xef\x8f\x81"	// U+f3c1
#define ICON_FA_LONG_ARROW_ALT_DOWN L"\xef\x8c\x89"	// U+f309
#define ICON_FA_LONG_ARROW_ALT_LEFT L"\xef\x8c\x8a"	// U+f30a
#define ICON_FA_LONG_ARROW_ALT_RIGHT L"\xef\x8c\x8b"	// U+f30b
#define ICON_FA_LONG_ARROW_ALT_UP L"\xef\x8c\x8c"	// U+f30c
#define ICON_FA_LOW_VISION L"\xef\x8a\xa8"	// U+f2a8
#define ICON_FA_LUGGAGE_CART L"\xef\x96\x9d"	// U+f59d
#define ICON_FA_LUNGS L"\xef\x98\x84"	// U+f604
#define ICON_FA_LUNGS_VIRUS L"\xee\x81\xa7"	// U+e067
#define ICON_FA_MAGIC L"\xef\x83\x90"	// U+f0d0
#define ICON_FA_MAGNET L"\xef\x81\xb6"	// U+f076
#define ICON_FA_MAIL_BULK L"\xef\x99\xb4"	// U+f674
#define ICON_FA_MALE L"\xef\x86\x83"	// U+f183
#define ICON_FA_MAP L"\xef\x89\xb9"	// U+f279
#define ICON_FA_MAP_MARKED L"\xef\x96\x9f"	// U+f59f
#define ICON_FA_MAP_MARKED_ALT L"\xef\x96\xa0"	// U+f5a0
#define ICON_FA_MAP_MARKER L"\xef\x81\x81"	// U+f041
#define ICON_FA_MAP_MARKER_ALT L"\xef\x8f\x85"	// U+f3c5
#define ICON_FA_MAP_PIN L"\xef\x89\xb6"	// U+f276
#define ICON_FA_MAP_SIGNS L"\xef\x89\xb7"	// U+f277
#define ICON_FA_MARKER L"\xef\x96\xa1"	// U+f5a1
#define ICON_FA_MARS L"\xef\x88\xa2"	// U+f222
#define ICON_FA_MARS_DOUBLE L"\xef\x88\xa7"	// U+f227
#define ICON_FA_MARS_STROKE L"\xef\x88\xa9"	// U+f229
#define ICON_FA_MARS_STROKE_H L"\xef\x88\xab"	// U+f22b
#define ICON_FA_MARS_STROKE_V L"\xef\x88\xaa"	// U+f22a
#define ICON_FA_MASK L"\xef\x9b\xba"	// U+f6fa
#define ICON_FA_MEDAL L"\xef\x96\xa2"	// U+f5a2
#define ICON_FA_MEDKIT L"\xef\x83\xba"	// U+f0fa
#define ICON_FA_MEH L"\xef\x84\x9a"	// U+f11a
#define ICON_FA_MEH_BLANK L"\xef\x96\xa4"	// U+f5a4
#define ICON_FA_MEH_ROLLING_EYES L"\xef\x96\xa5"	// U+f5a5
#define ICON_FA_MEMORY L"\xef\x94\xb8"	// U+f538
#define ICON_FA_MENORAH L"\xef\x99\xb6"	// U+f676
#define ICON_FA_MERCURY L"\xef\x88\xa3"	// U+f223
#define ICON_FA_METEOR L"\xef\x9d\x93"	// U+f753
#define ICON_FA_MICROCHIP L"\xef\x8b\x9b"	// U+f2db
#define ICON_FA_MICROPHONE L"\xef\x84\xb0"	// U+f130
#define ICON_FA_MICROPHONE_ALT L"\xef\x8f\x89"	// U+f3c9
#define ICON_FA_MICROPHONE_ALT_SLASH L"\xef\x94\xb9"	// U+f539
#define ICON_FA_MICROPHONE_SLASH L"\xef\x84\xb1"	// U+f131
#define ICON_FA_MICROSCOPE L"\xef\x98\x90"	// U+f610
#define ICON_FA_MINUS L"\xef\x81\xa8"	// U+f068
#define ICON_FA_MINUS_CIRCLE L"\xef\x81\x96"	// U+f056
#define ICON_FA_MINUS_SQUARE L"\xef\x85\x86"	// U+f146
#define ICON_FA_MITTEN L"\xef\x9e\xb5"	// U+f7b5
#define ICON_FA_MOBILE L"\xef\x84\x8b"	// U+f10b
#define ICON_FA_MOBILE_ALT L"\xef\x8f\x8d"	// U+f3cd
#define ICON_FA_MONEY_BILL L"\xef\x83\x96"	// U+f0d6
#define ICON_FA_MONEY_BILL_ALT L"\xef\x8f\x91"	// U+f3d1
#define ICON_FA_MONEY_BILL_WAVE L"\xef\x94\xba"	// U+f53a
#define ICON_FA_MONEY_BILL_WAVE_ALT L"\xef\x94\xbb"	// U+f53b
#define ICON_FA_MONEY_CHECK L"\xef\x94\xbc"	// U+f53c
#define ICON_FA_MONEY_CHECK_ALT L"\xef\x94\xbd"	// U+f53d
#define ICON_FA_MONUMENT L"\xef\x96\xa6"	// U+f5a6
#define ICON_FA_MOON L"\xef\x86\x86"	// U+f186
#define ICON_FA_MORTAR_PESTLE L"\xef\x96\xa7"	// U+f5a7
#define ICON_FA_MOSQUE L"\xef\x99\xb8"	// U+f678
#define ICON_FA_MOTORCYCLE L"\xef\x88\x9c"	// U+f21c
#define ICON_FA_MOUNTAIN L"\xef\x9b\xbc"	// U+f6fc
#define ICON_FA_MOUSE L"\xef\xa3\x8c"	// U+f8cc
#define ICON_FA_MOUSE_POINTER L"\xef\x89\x85"	// U+f245
#define ICON_FA_MUG_HOT L"\xef\x9e\xb6"	// U+f7b6
#define ICON_FA_MUSIC L"\xef\x80\x81"	// U+f001
#define ICON_FA_NETWORK_WIRED L"\xef\x9b\xbf"	// U+f6ff
#define ICON_FA_NEUTER L"\xef\x88\xac"	// U+f22c
#define ICON_FA_NEWSPAPER L"\xef\x87\xaa"	// U+f1ea
#define ICON_FA_NOT_EQUAL L"\xef\x94\xbe"	// U+f53e
#define ICON_FA_NOTES_MEDICAL L"\xef\x92\x81"	// U+f481
#define ICON_FA_OBJECT_GROUP L"\xef\x89\x87"	// U+f247
#define ICON_FA_OBJECT_UNGROUP L"\xef\x89\x88"	// U+f248
#define ICON_FA_OIL_CAN L"\xef\x98\x93"	// U+f613
#define ICON_FA_OM L"\xef\x99\xb9"	// U+f679
#define ICON_FA_OTTER L"\xef\x9c\x80"	// U+f700
#define ICON_FA_OUTDENT L"\xef\x80\xbb"	// U+f03b
#define ICON_FA_PAGER L"\xef\xa0\x95"	// U+f815
#define ICON_FA_PAINT_BRUSH L"\xef\x87\xbc"	// U+f1fc
#define ICON_FA_PAINT_ROLLER L"\xef\x96\xaa"	// U+f5aa
#define ICON_FA_PALETTE L"\xef\x94\xbf"	// U+f53f
#define ICON_FA_PALLET L"\xef\x92\x82"	// U+f482
#define ICON_FA_PAPER_PLANE L"\xef\x87\x98"	// U+f1d8
#define ICON_FA_PAPERCLIP L"\xef\x83\x86"	// U+f0c6
#define ICON_FA_PARACHUTE_BOX L"\xef\x93\x8d"	// U+f4cd
#define ICON_FA_PARAGRAPH L"\xef\x87\x9d"	// U+f1dd
#define ICON_FA_PARKING L"\xef\x95\x80"	// U+f540
#define ICON_FA_PASSPORT L"\xef\x96\xab"	// U+f5ab
#define ICON_FA_PASTAFARIANISM L"\xef\x99\xbb"	// U+f67b
#define ICON_FA_PASTE L"\xef\x83\xaa"	// U+f0ea
#define ICON_FA_PAUSE L"\xef\x81\x8c"	// U+f04c
#define ICON_FA_PAUSE_CIRCLE L"\xef\x8a\x8b"	// U+f28b
#define ICON_FA_PAW L"\xef\x86\xb0"	// U+f1b0
#define ICON_FA_PEACE L"\xef\x99\xbc"	// U+f67c
#define ICON_FA_PEN L"\xef\x8c\x84"	// U+f304
#define ICON_FA_PEN_ALT L"\xef\x8c\x85"	// U+f305
#define ICON_FA_PEN_FANCY L"\xef\x96\xac"	// U+f5ac
#define ICON_FA_PEN_NIB L"\xef\x96\xad"	// U+f5ad
#define ICON_FA_PEN_SQUARE L"\xef\x85\x8b"	// U+f14b
#define ICON_FA_PENCIL_ALT L"\xef\x8c\x83"	// U+f303
#define ICON_FA_PENCIL_RULER L"\xef\x96\xae"	// U+f5ae
#define ICON_FA_PEOPLE_ARROWS L"\xee\x81\xa8"	// U+e068
#define ICON_FA_PEOPLE_CARRY L"\xef\x93\x8e"	// U+f4ce
#define ICON_FA_PEPPER_HOT L"\xef\xa0\x96"	// U+f816
#define ICON_FA_PERCENT L"\xef\x8a\x95"	// U+f295
#define ICON_FA_PERCENTAGE L"\xef\x95\x81"	// U+f541
#define ICON_FA_PERSON_BOOTH L"\xef\x9d\x96"	// U+f756
#define ICON_FA_PHONE L"\xef\x82\x95"	// U+f095
#define ICON_FA_PHONE_ALT L"\xef\xa1\xb9"	// U+f879
#define ICON_FA_PHONE_SLASH L"\xef\x8f\x9d"	// U+f3dd
#define ICON_FA_PHONE_SQUARE L"\xef\x82\x98"	// U+f098
#define ICON_FA_PHONE_SQUARE_ALT L"\xef\xa1\xbb"	// U+f87b
#define ICON_FA_PHONE_VOLUME L"\xef\x8a\xa0"	// U+f2a0
#define ICON_FA_PHOTO_VIDEO L"\xef\xa1\xbc"	// U+f87c
#define ICON_FA_PIGGY_BANK L"\xef\x93\x93"	// U+f4d3
#define ICON_FA_PILLS L"\xef\x92\x84"	// U+f484
#define ICON_FA_PIZZA_SLICE L"\xef\xa0\x98"	// U+f818
#define ICON_FA_PLACE_OF_WORSHIP L"\xef\x99\xbf"	// U+f67f
#define ICON_FA_PLANE L"\xef\x81\xb2"	// U+f072
#define ICON_FA_PLANE_ARRIVAL L"\xef\x96\xaf"	// U+f5af
#define ICON_FA_PLANE_DEPARTURE L"\xef\x96\xb0"	// U+f5b0
#define ICON_FA_PLANE_SLASH L"\xee\x81\xa9"	// U+e069
#define ICON_FA_PLAY L"\xef\x81\x8b"	// U+f04b
#define ICON_FA_PLAY_CIRCLE L"\xef\x85\x84"	// U+f144
#define ICON_FA_PLUG L"\xef\x87\xa6"	// U+f1e6
#define ICON_FA_PLUS L"\xef\x81\xa7"	// U+f067
#define ICON_FA_PLUS_CIRCLE L"\xef\x81\x95"	// U+f055
#define ICON_FA_PLUS_SQUARE L"\xef\x83\xbe"	// U+f0fe
#define ICON_FA_PODCAST L"\xef\x8b\x8e"	// U+f2ce
#define ICON_FA_POLL L"\xef\x9a\x81"	// U+f681
#define ICON_FA_POLL_H L"\xef\x9a\x82"	// U+f682
#define ICON_FA_POO L"\xef\x8b\xbe"	// U+f2fe
#define ICON_FA_POO_STORM L"\xef\x9d\x9a"	// U+f75a
#define ICON_FA_POOP L"\xef\x98\x99"	// U+f619
#define ICON_FA_PORTRAIT L"\xef\x8f\xa0"	// U+f3e0
#define ICON_FA_POUND_SIGN L"\xef\x85\x94"	// U+f154
#define ICON_FA_POWER_OFF L"\xef\x80\x91"	// U+f011
#define ICON_FA_PRAY L"\xef\x9a\x83"	// U+f683
#define ICON_FA_PRAYING_HANDS L"\xef\x9a\x84"	// U+f684
#define ICON_FA_PRESCRIPTION L"\xef\x96\xb1"	// U+f5b1
#define ICON_FA_PRESCRIPTION_BOTTLE L"\xef\x92\x85"	// U+f485
#define ICON_FA_PRESCRIPTION_BOTTLE_ALT L"\xef\x92\x86"	// U+f486
#define ICON_FA_PRINT L"\xef\x80\xaf"	// U+f02f
#define ICON_FA_PROCEDURES L"\xef\x92\x87"	// U+f487
#define ICON_FA_PROJECT_DIAGRAM L"\xef\x95\x82"	// U+f542
#define ICON_FA_PUMP_MEDICAL L"\xee\x81\xaa"	// U+e06a
#define ICON_FA_PUMP_SOAP L"\xee\x81\xab"	// U+e06b
#define ICON_FA_PUZZLE_PIECE L"\xef\x84\xae"	// U+f12e
#define ICON_FA_QRCODE L"\xef\x80\xa9"	// U+f029
#define ICON_FA_QUESTION L"\xef\x84\xa8"	// U+f128
#define ICON_FA_QUESTION_CIRCLE L"\xef\x81\x99"	// U+f059
#define ICON_FA_QUIDDITCH L"\xef\x91\x98"	// U+f458
#define ICON_FA_QUOTE_LEFT L"\xef\x84\x8d"	// U+f10d
#define ICON_FA_QUOTE_RIGHT L"\xef\x84\x8e"	// U+f10e
#define ICON_FA_QURAN L"\xef\x9a\x87"	// U+f687
#define ICON_FA_RADIATION L"\xef\x9e\xb9"	// U+f7b9
#define ICON_FA_RADIATION_ALT L"\xef\x9e\xba"	// U+f7ba
#define ICON_FA_RAINBOW L"\xef\x9d\x9b"	// U+f75b
#define ICON_FA_RANDOM L"\xef\x81\xb4"	// U+f074
#define ICON_FA_RECEIPT L"\xef\x95\x83"	// U+f543
#define ICON_FA_RECORD_VINYL L"\xef\xa3\x99"	// U+f8d9
#define ICON_FA_RECYCLE L"\xef\x86\xb8"	// U+f1b8
#define ICON_FA_REDO L"\xef\x80\x9e"	// U+f01e
#define ICON_FA_REDO_ALT L"\xef\x8b\xb9"	// U+f2f9
#define ICON_FA_REGISTERED L"\xef\x89\x9d"	// U+f25d
#define ICON_FA_REMOVE_FORMAT L"\xef\xa1\xbd"	// U+f87d
#define ICON_FA_REPLY L"\xef\x8f\xa5"	// U+f3e5
#define ICON_FA_REPLY_ALL L"\xef\x84\xa2"	// U+f122
#define ICON_FA_REPUBLICAN L"\xef\x9d\x9e"	// U+f75e
#define ICON_FA_RESTROOM L"\xef\x9e\xbd"	// U+f7bd
#define ICON_FA_RETWEET L"\xef\x81\xb9"	// U+f079
#define ICON_FA_RIBBON L"\xef\x93\x96"	// U+f4d6
#define ICON_FA_RING L"\xef\x9c\x8b"	// U+f70b
#define ICON_FA_ROAD L"\xef\x80\x98"	// U+f018
#define ICON_FA_ROBOT L"\xef\x95\x84"	// U+f544
#define ICON_FA_ROCKET L"\xef\x84\xb5"	// U+f135
#define ICON_FA_ROUTE L"\xef\x93\x97"	// U+f4d7
#define ICON_FA_RSS L"\xef\x82\x9e"	// U+f09e
#define ICON_FA_RSS_SQUARE L"\xef\x85\x83"	// U+f143
#define ICON_FA_RUBLE_SIGN L"\xef\x85\x98"	// U+f158
#define ICON_FA_RULER L"\xef\x95\x85"	// U+f545
#define ICON_FA_RULER_COMBINED L"\xef\x95\x86"	// U+f546
#define ICON_FA_RULER_HORIZONTAL L"\xef\x95\x87"	// U+f547
#define ICON_FA_RULER_VERTICAL L"\xef\x95\x88"	// U+f548
#define ICON_FA_RUNNING L"\xef\x9c\x8c"	// U+f70c
#define ICON_FA_RUPEE_SIGN L"\xef\x85\x96"	// U+f156
#define ICON_FA_SAD_CRY L"\xef\x96\xb3"	// U+f5b3
#define ICON_FA_SAD_TEAR L"\xef\x96\xb4"	// U+f5b4
#define ICON_FA_SATELLITE L"\xef\x9e\xbf"	// U+f7bf
#define ICON_FA_SATELLITE_DISH L"\xef\x9f\x80"	// U+f7c0
#define ICON_FA_SAVE L"\xef\x83\x87"	// U+f0c7
#define ICON_FA_SCHOOL L"\xef\x95\x89"	// U+f549
#define ICON_FA_SCREWDRIVER L"\xef\x95\x8a"	// U+f54a
#define ICON_FA_SCROLL L"\xef\x9c\x8e"	// U+f70e
#define ICON_FA_SD_CARD L"\xef\x9f\x82"	// U+f7c2
#define ICON_FA_SEARCH L"\xef\x80\x82"	// U+f002
#define ICON_FA_SEARCH_DOLLAR L"\xef\x9a\x88"	// U+f688
#define ICON_FA_SEARCH_LOCATION L"\xef\x9a\x89"	// U+f689
#define ICON_FA_SEARCH_MINUS L"\xef\x80\x90"	// U+f010
#define ICON_FA_SEARCH_PLUS L"\xef\x80\x8e"	// U+f00e
#define ICON_FA_SEEDLING L"\xef\x93\x98"	// U+f4d8
#define ICON_FA_SERVER L"\xef\x88\xb3"	// U+f233
#define ICON_FA_SHAPES L"\xef\x98\x9f"	// U+f61f
#define ICON_FA_SHARE L"\xef\x81\xa4"	// U+f064
#define ICON_FA_SHARE_ALT L"\xef\x87\xa0"	// U+f1e0
#define ICON_FA_SHARE_ALT_SQUARE L"\xef\x87\xa1"	// U+f1e1
#define ICON_FA_SHARE_SQUARE L"\xef\x85\x8d"	// U+f14d
#define ICON_FA_SHEKEL_SIGN L"\xef\x88\x8b"	// U+f20b
#define ICON_FA_SHIELD_ALT L"\xef\x8f\xad"	// U+f3ed
#define ICON_FA_SHIELD_VIRUS L"\xee\x81\xac"	// U+e06c
#define ICON_FA_SHIP L"\xef\x88\x9a"	// U+f21a
#define ICON_FA_SHIPPING_FAST L"\xef\x92\x8b"	// U+f48b
#define ICON_FA_SHOE_PRINTS L"\xef\x95\x8b"	// U+f54b
#define ICON_FA_SHOPPING_BAG L"\xef\x8a\x90"	// U+f290
#define ICON_FA_SHOPPING_BASKET L"\xef\x8a\x91"	// U+f291
#define ICON_FA_SHOPPING_CART L"\xef\x81\xba"	// U+f07a
#define ICON_FA_SHOWER L"\xef\x8b\x8c"	// U+f2cc
#define ICON_FA_SHUTTLE_VAN L"\xef\x96\xb6"	// U+f5b6
#define ICON_FA_SIGN L"\xef\x93\x99"	// U+f4d9
#define ICON_FA_SIGN_IN_ALT L"\xef\x8b\xb6"	// U+f2f6
#define ICON_FA_SIGN_LANGUAGE L"\xef\x8a\xa7"	// U+f2a7
#define ICON_FA_SIGN_OUT_ALT L"\xef\x8b\xb5"	// U+f2f5
#define ICON_FA_SIGNAL L"\xef\x80\x92"	// U+f012
#define ICON_FA_SIGNATURE L"\xef\x96\xb7"	// U+f5b7
#define ICON_FA_SIM_CARD L"\xef\x9f\x84"	// U+f7c4
#define ICON_FA_SINK L"\xee\x81\xad"	// U+e06d
#define ICON_FA_SITEMAP L"\xef\x83\xa8"	// U+f0e8
#define ICON_FA_SKATING L"\xef\x9f\x85"	// U+f7c5
#define ICON_FA_SKIING L"\xef\x9f\x89"	// U+f7c9
#define ICON_FA_SKIING_NORDIC L"\xef\x9f\x8a"	// U+f7ca
#define ICON_FA_SKULL L"\xef\x95\x8c"	// U+f54c
#define ICON_FA_SKULL_CROSSBONES L"\xef\x9c\x94"	// U+f714
#define ICON_FA_SLASH L"\xef\x9c\x95"	// U+f715
#define ICON_FA_SLEIGH L"\xef\x9f\x8c"	// U+f7cc
#define ICON_FA_SLIDERS_H L"\xef\x87\x9e"	// U+f1de
#define ICON_FA_SMILE L"\xef\x84\x98"	// U+f118
#define ICON_FA_SMILE_BEAM L"\xef\x96\xb8"	// U+f5b8
#define ICON_FA_SMILE_WINK L"\xef\x93\x9a"	// U+f4da
#define ICON_FA_SMOG L"\xef\x9d\x9f"	// U+f75f
#define ICON_FA_SMOKING L"\xef\x92\x8d"	// U+f48d
#define ICON_FA_SMOKING_BAN L"\xef\x95\x8d"	// U+f54d
#define ICON_FA_SMS L"\xef\x9f\x8d"	// U+f7cd
#define ICON_FA_SNOWBOARDING L"\xef\x9f\x8e"	// U+f7ce
#define ICON_FA_SNOWFLAKE L"\xef\x8b\x9c"	// U+f2dc
#define ICON_FA_SNOWMAN L"\xef\x9f\x90"	// U+f7d0
#define ICON_FA_SNOWPLOW L"\xef\x9f\x92"	// U+f7d2
#define ICON_FA_SOAP L"\xee\x81\xae"	// U+e06e
#define ICON_FA_SOCKS L"\xef\x9a\x96"	// U+f696
#define ICON_FA_SOLAR_PANEL L"\xef\x96\xba"	// U+f5ba
#define ICON_FA_SORT L"\xef\x83\x9c"	// U+f0dc
#define ICON_FA_SORT_ALPHA_DOWN L"\xef\x85\x9d"	// U+f15d
#define ICON_FA_SORT_ALPHA_DOWN_ALT L"\xef\xa2\x81"	// U+f881
#define ICON_FA_SORT_ALPHA_UP L"\xef\x85\x9e"	// U+f15e
#define ICON_FA_SORT_ALPHA_UP_ALT L"\xef\xa2\x82"	// U+f882
#define ICON_FA_SORT_AMOUNT_DOWN L"\xef\x85\xa0"	// U+f160
#define ICON_FA_SORT_AMOUNT_DOWN_ALT L"\xef\xa2\x84"	// U+f884
#define ICON_FA_SORT_AMOUNT_UP L"\xef\x85\xa1"	// U+f161
#define ICON_FA_SORT_AMOUNT_UP_ALT L"\xef\xa2\x85"	// U+f885
#define ICON_FA_SORT_DOWN L"\xef\x83\x9d"	// U+f0dd
#define ICON_FA_SORT_NUMERIC_DOWN L"\xef\x85\xa2"	// U+f162
#define ICON_FA_SORT_NUMERIC_DOWN_ALT L"\xef\xa2\x86"	// U+f886
#define ICON_FA_SORT_NUMERIC_UP L"\xef\x85\xa3"	// U+f163
#define ICON_FA_SORT_NUMERIC_UP_ALT L"\xef\xa2\x87"	// U+f887
#define ICON_FA_SORT_UP L"\xef\x83\x9e"	// U+f0de
#define ICON_FA_SPA L"\xef\x96\xbb"	// U+f5bb
#define ICON_FA_SPACE_SHUTTLE L"\xef\x86\x97"	// U+f197
#define ICON_FA_SPELL_CHECK L"\xef\xa2\x91"	// U+f891
#define ICON_FA_SPIDER L"\xef\x9c\x97"	// U+f717
#define ICON_FA_SPINNER L"\xef\x84\x90"	// U+f110
#define ICON_FA_SPLOTCH L"\xef\x96\xbc"	// U+f5bc
#define ICON_FA_SPRAY_CAN L"\xef\x96\xbd"	// U+f5bd
#define ICON_FA_SQUARE L"\xef\x83\x88"	// U+f0c8
#define ICON_FA_SQUARE_FULL L"\xef\x91\x9c"	// U+f45c
#define ICON_FA_SQUARE_ROOT_ALT L"\xef\x9a\x98"	// U+f698
#define ICON_FA_STAMP L"\xef\x96\xbf"	// U+f5bf
#define ICON_FA_STAR L"\xef\x80\x85"	// U+f005
#define ICON_FA_STAR_AND_CRESCENT L"\xef\x9a\x99"	// U+f699
#define ICON_FA_STAR_HALF L"\xef\x82\x89"	// U+f089
#define ICON_FA_STAR_HALF_ALT L"\xef\x97\x80"	// U+f5c0
#define ICON_FA_STAR_OF_DAVID L"\xef\x9a\x9a"	// U+f69a
#define ICON_FA_STAR_OF_LIFE L"\xef\x98\xa1"	// U+f621
#define ICON_FA_STEP_BACKWARD L"\xef\x81\x88"	// U+f048
#define ICON_FA_STEP_FORWARD L"\xef\x81\x91"	// U+f051
#define ICON_FA_STETHOSCOPE L"\xef\x83\xb1"	// U+f0f1
#define ICON_FA_STICKY_NOTE L"\xef\x89\x89"	// U+f249
#define ICON_FA_STOP L"\xef\x81\x8d"	// U+f04d
#define ICON_FA_STOP_CIRCLE L"\xef\x8a\x8d"	// U+f28d
#define ICON_FA_STOPWATCH L"\xef\x8b\xb2"	// U+f2f2
#define ICON_FA_STOPWATCH_20 L"\xee\x81\xaf"	// U+e06f
#define ICON_FA_STORE L"\xef\x95\x8e"	// U+f54e
#define ICON_FA_STORE_ALT L"\xef\x95\x8f"	// U+f54f
#define ICON_FA_STORE_ALT_SLASH L"\xee\x81\xb0"	// U+e070
#define ICON_FA_STORE_SLASH L"\xee\x81\xb1"	// U+e071
#define ICON_FA_STREAM L"\xef\x95\x90"	// U+f550
#define ICON_FA_STREET_VIEW L"\xef\x88\x9d"	// U+f21d
#define ICON_FA_STRIKETHROUGH L"\xef\x83\x8c"	// U+f0cc
#define ICON_FA_STROOPWAFEL L"\xef\x95\x91"	// U+f551
#define ICON_FA_SUBSCRIPT L"\xef\x84\xac"	// U+f12c
#define ICON_FA_SUBWAY L"\xef\x88\xb9"	// U+f239
#define ICON_FA_SUITCASE L"\xef\x83\xb2"	// U+f0f2
#define ICON_FA_SUITCASE_ROLLING L"\xef\x97\x81"	// U+f5c1
#define ICON_FA_SUN L"\xef\x86\x85"	// U+f185
#define ICON_FA_SUPERSCRIPT L"\xef\x84\xab"	// U+f12b
#define ICON_FA_SURPRISE L"\xef\x97\x82"	// U+f5c2
#define ICON_FA_SWATCHBOOK L"\xef\x97\x83"	// U+f5c3
#define ICON_FA_SWIMMER L"\xef\x97\x84"	// U+f5c4
#define ICON_FA_SWIMMING_POOL L"\xef\x97\x85"	// U+f5c5
#define ICON_FA_SYNAGOGUE L"\xef\x9a\x9b"	// U+f69b
#define ICON_FA_SYNC L"\xef\x80\xa1"	// U+f021
#define ICON_FA_SYNC_ALT L"\xef\x8b\xb1"	// U+f2f1
#define ICON_FA_SYRINGE L"\xef\x92\x8e"	// U+f48e
#define ICON_FA_TABLE L"\xef\x83\x8e"	// U+f0ce
#define ICON_FA_TABLE_TENNIS L"\xef\x91\x9d"	// U+f45d
#define ICON_FA_TABLET L"\xef\x84\x8a"	// U+f10a
#define ICON_FA_TABLET_ALT L"\xef\x8f\xba"	// U+f3fa
#define ICON_FA_TABLETS L"\xef\x92\x90"	// U+f490
#define ICON_FA_TACHOMETER_ALT L"\xef\x8f\xbd"	// U+f3fd
#define ICON_FA_TAG L"\xef\x80\xab"	// U+f02b
#define ICON_FA_TAGS L"\xef\x80\xac"	// U+f02c
#define ICON_FA_TAPE L"\xef\x93\x9b"	// U+f4db
#define ICON_FA_TASKS L"\xef\x82\xae"	// U+f0ae
#define ICON_FA_TAXI L"\xef\x86\xba"	// U+f1ba
#define ICON_FA_TEETH L"\xef\x98\xae"	// U+f62e
#define ICON_FA_TEETH_OPEN L"\xef\x98\xaf"	// U+f62f
#define ICON_FA_TEMPERATURE_HIGH L"\xef\x9d\xa9"	// U+f769
#define ICON_FA_TEMPERATURE_LOW L"\xef\x9d\xab"	// U+f76b
#define ICON_FA_TENGE L"\xef\x9f\x97"	// U+f7d7
#define ICON_FA_TERMINAL L"\xef\x84\xa0"	// U+f120
#define ICON_FA_TEXT_HEIGHT L"\xef\x80\xb4"	// U+f034
#define ICON_FA_TEXT_WIDTH L"\xef\x80\xb5"	// U+f035
#define ICON_FA_TH L"\xef\x80\x8a"	// U+f00a
#define ICON_FA_TH_LARGE L"\xef\x80\x89"	// U+f009
#define ICON_FA_TH_LIST L"\xef\x80\x8b"	// U+f00b
#define ICON_FA_THEATER_MASKS L"\xef\x98\xb0"	// U+f630
#define ICON_FA_THERMOMETER L"\xef\x92\x91"	// U+f491
#define ICON_FA_THERMOMETER_EMPTY L"\xef\x8b\x8b"	// U+f2cb
#define ICON_FA_THERMOMETER_FULL L"\xef\x8b\x87"	// U+f2c7
#define ICON_FA_THERMOMETER_HALF L"\xef\x8b\x89"	// U+f2c9
#define ICON_FA_THERMOMETER_QUARTER L"\xef\x8b\x8a"	// U+f2ca
#define ICON_FA_THERMOMETER_THREE_QUARTERS L"\xef\x8b\x88"	// U+f2c8
#define ICON_FA_THUMBS_DOWN L"\xef\x85\xa5"	// U+f165
#define ICON_FA_THUMBS_UP L"\xef\x85\xa4"	// U+f164
#define ICON_FA_THUMBTACK L"\xef\x82\x8d"	// U+f08d
#define ICON_FA_TICKET_ALT L"\xef\x8f\xbf"	// U+f3ff
#define ICON_FA_TIMES L"\xef\x80\x8d"	// U+f00d
#define ICON_FA_TIMES_CIRCLE L"\xef\x81\x97"	// U+f057
#define ICON_FA_TINT L"\xef\x81\x83"	// U+f043
#define ICON_FA_TINT_SLASH L"\xef\x97\x87"	// U+f5c7
#define ICON_FA_TIRED L"\xef\x97\x88"	// U+f5c8
#define ICON_FA_TOGGLE_OFF L"\xef\x88\x84"	// U+f204
#define ICON_FA_TOGGLE_ON L"\xef\x88\x85"	// U+f205
#define ICON_FA_TOILET L"\xef\x9f\x98"	// U+f7d8
#define ICON_FA_TOILET_PAPER L"\xef\x9c\x9e"	// U+f71e
#define ICON_FA_TOILET_PAPER_SLASH L"\xee\x81\xb2"	// U+e072
#define ICON_FA_TOOLBOX L"\xef\x95\x92"	// U+f552
#define ICON_FA_TOOLS L"\xef\x9f\x99"	// U+f7d9
#define ICON_FA_TOOTH L"\xef\x97\x89"	// U+f5c9
#define ICON_FA_TORAH L"\xef\x9a\xa0"	// U+f6a0
#define ICON_FA_TORII_GATE L"\xef\x9a\xa1"	// U+f6a1
#define ICON_FA_TRACTOR L"\xef\x9c\xa2"	// U+f722
#define ICON_FA_TRADEMARK L"\xef\x89\x9c"	// U+f25c
#define ICON_FA_TRAFFIC_LIGHT L"\xef\x98\xb7"	// U+f637
#define ICON_FA_TRAILER L"\xee\x81\x81"	// U+e041
#define ICON_FA_TRAIN L"\xef\x88\xb8"	// U+f238
#define ICON_FA_TRAM L"\xef\x9f\x9a"	// U+f7da
#define ICON_FA_TRANSGENDER L"\xef\x88\xa4"	// U+f224
#define ICON_FA_TRANSGENDER_ALT L"\xef\x88\xa5"	// U+f225
#define ICON_FA_TRASH L"\xef\x87\xb8"	// U+f1f8
#define ICON_FA_TRASH_ALT L"\xef\x8b\xad"	// U+f2ed
#define ICON_FA_TRASH_RESTORE L"\xef\xa0\xa9"	// U+f829
#define ICON_FA_TRASH_RESTORE_ALT L"\xef\xa0\xaa"	// U+f82a
#define ICON_FA_TREE L"\xef\x86\xbb"	// U+f1bb
#define ICON_FA_TROPHY L"\xef\x82\x91"	// U+f091
#define ICON_FA_TRUCK L"\xef\x83\x91"	// U+f0d1
#define ICON_FA_TRUCK_LOADING L"\xef\x93\x9e"	// U+f4de
#define ICON_FA_TRUCK_MONSTER L"\xef\x98\xbb"	// U+f63b
#define ICON_FA_TRUCK_MOVING L"\xef\x93\x9f"	// U+f4df
#define ICON_FA_TRUCK_PICKUP L"\xef\x98\xbc"	// U+f63c
#define ICON_FA_TSHIRT L"\xef\x95\x93"	// U+f553
#define ICON_FA_TTY L"\xef\x87\xa4"	// U+f1e4
#define ICON_FA_TV L"\xef\x89\xac"	// U+f26c
#define ICON_FA_UMBRELLA L"\xef\x83\xa9"	// U+f0e9
#define ICON_FA_UMBRELLA_BEACH L"\xef\x97\x8a"	// U+f5ca
#define ICON_FA_UNDERLINE L"\xef\x83\x8d"	// U+f0cd
#define ICON_FA_UNDO L"\xef\x83\xa2"	// U+f0e2
#define ICON_FA_UNDO_ALT L"\xef\x8b\xaa"	// U+f2ea
#define ICON_FA_UNIVERSAL_ACCESS L"\xef\x8a\x9a"	// U+f29a
#define ICON_FA_UNIVERSITY L"\xef\x86\x9c"	// U+f19c
#define ICON_FA_UNLINK L"\xef\x84\xa7"	// U+f127
#define ICON_FA_UNLOCK L"\xef\x82\x9c"	// U+f09c
#define ICON_FA_UNLOCK_ALT L"\xef\x84\xbe"	// U+f13e
#define ICON_FA_UPLOAD L"\xef\x82\x93"	// U+f093
#define ICON_FA_USER L"\xef\x80\x87"	// U+f007
#define ICON_FA_USER_ALT L"\xef\x90\x86"	// U+f406
#define ICON_FA_USER_ALT_SLASH L"\xef\x93\xba"	// U+f4fa
#define ICON_FA_USER_ASTRONAUT L"\xef\x93\xbb"	// U+f4fb
#define ICON_FA_USER_CHECK L"\xef\x93\xbc"	// U+f4fc
#define ICON_FA_USER_CIRCLE L"\xef\x8a\xbd"	// U+f2bd
#define ICON_FA_USER_CLOCK L"\xef\x93\xbd"	// U+f4fd
#define ICON_FA_USER_COG L"\xef\x93\xbe"	// U+f4fe
#define ICON_FA_USER_EDIT L"\xef\x93\xbf"	// U+f4ff
#define ICON_FA_USER_FRIENDS L"\xef\x94\x80"	// U+f500
#define ICON_FA_USER_GRADUATE L"\xef\x94\x81"	// U+f501
#define ICON_FA_USER_INJURED L"\xef\x9c\xa8"	// U+f728
#define ICON_FA_USER_LOCK L"\xef\x94\x82"	// U+f502
#define ICON_FA_USER_MD L"\xef\x83\xb0"	// U+f0f0
#define ICON_FA_USER_MINUS L"\xef\x94\x83"	// U+f503
#define ICON_FA_USER_NINJA L"\xef\x94\x84"	// U+f504
#define ICON_FA_USER_NURSE L"\xef\xa0\xaf"	// U+f82f
#define ICON_FA_USER_PLUS L"\xef\x88\xb4"	// U+f234
#define ICON_FA_USER_SECRET L"\xef\x88\x9b"	// U+f21b
#define ICON_FA_USER_SHIELD L"\xef\x94\x85"	// U+f505
#define ICON_FA_USER_SLASH L"\xef\x94\x86"	// U+f506
#define ICON_FA_USER_TAG L"\xef\x94\x87"	// U+f507
#define ICON_FA_USER_TIE L"\xef\x94\x88"	// U+f508
#define ICON_FA_USER_TIMES L"\xef\x88\xb5"	// U+f235
#define ICON_FA_USERS L"\xef\x83\x80"	// U+f0c0
#define ICON_FA_USERS_COG L"\xef\x94\x89"	// U+f509
#define ICON_FA_USERS_SLASH L"\xee\x81\xb3"	// U+e073
#define ICON_FA_UTENSIL_SPOON L"\xef\x8b\xa5"	// U+f2e5
#define ICON_FA_UTENSILS L"\xef\x8b\xa7"	// U+f2e7
#define ICON_FA_VECTOR_SQUARE L"\xef\x97\x8b"	// U+f5cb
#define ICON_FA_VENUS L"\xef\x88\xa1"	// U+f221
#define ICON_FA_VENUS_DOUBLE L"\xef\x88\xa6"	// U+f226
#define ICON_FA_VENUS_MARS L"\xef\x88\xa8"	// U+f228
#define ICON_FA_VEST L"\xee\x82\x85"	// U+e085
#define ICON_FA_VEST_PATCHES L"\xee\x82\x86"	// U+e086
#define ICON_FA_VIAL L"\xef\x92\x92"	// U+f492
#define ICON_FA_VIALS L"\xef\x92\x93"	// U+f493
#define ICON_FA_VIDEO L"\xef\x80\xbd"	// U+f03d
#define ICON_FA_VIDEO_SLASH L"\xef\x93\xa2"	// U+f4e2
#define ICON_FA_VIHARA L"\xef\x9a\xa7"	// U+f6a7
#define ICON_FA_VIRUS L"\xee\x81\xb4"	// U+e074
#define ICON_FA_VIRUS_SLASH L"\xee\x81\xb5"	// U+e075
#define ICON_FA_VIRUSES L"\xee\x81\xb6"	// U+e076
#define ICON_FA_VOICEMAIL L"\xef\xa2\x97"	// U+f897
#define ICON_FA_VOLLEYBALL_BALL L"\xef\x91\x9f"	// U+f45f
#define ICON_FA_VOLUME_DOWN L"\xef\x80\xa7"	// U+f027
#define ICON_FA_VOLUME_MUTE L"\xef\x9a\xa9"	// U+f6a9
#define ICON_FA_VOLUME_OFF L"\xef\x80\xa6"	// U+f026
#define ICON_FA_VOLUME_UP L"\xef\x80\xa8"	// U+f028
#define ICON_FA_VOTE_YEA L"\xef\x9d\xb2"	// U+f772
#define ICON_FA_VR_CARDBOARD L"\xef\x9c\xa9"	// U+f729
#define ICON_FA_WALKING L"\xef\x95\x94"	// U+f554
#define ICON_FA_WALLET L"\xef\x95\x95"	// U+f555
#define ICON_FA_WAREHOUSE L"\xef\x92\x94"	// U+f494
#define ICON_FA_WATER L"\xef\x9d\xb3"	// U+f773
#define ICON_FA_WAVE_SQUARE L"\xef\xa0\xbe"	// U+f83e
#define ICON_FA_WEIGHT L"\xef\x92\x96"	// U+f496
#define ICON_FA_WEIGHT_HANGING L"\xef\x97\x8d"	// U+f5cd
#define ICON_FA_WHEELCHAIR L"\xef\x86\x93"	// U+f193
#define ICON_FA_WIFI L"\xef\x87\xab"	// U+f1eb
#define ICON_FA_WIND L"\xef\x9c\xae"	// U+f72e
#define ICON_FA_WINDOW_CLOSE L"\xef\x90\x90"	// U+f410
#define ICON_FA_WINDOW_MAXIMIZE L"\xef\x8b\x90"	// U+f2d0
#define ICON_FA_WINDOW_MINIMIZE L"\xef\x8b\x91"	// U+f2d1
#define ICON_FA_WINDOW_RESTORE L"\xef\x8b\x92"	// U+f2d2
#define ICON_FA_WINE_BOTTLE L"\xef\x9c\xaf"	// U+f72f
#define ICON_FA_WINE_GLASS L"\xef\x93\xa3"	// U+f4e3
#define ICON_FA_WINE_GLASS_ALT L"\xef\x97\x8e"	// U+f5ce
#define ICON_FA_WON_SIGN L"\xef\x85\x99"	// U+f159
#define ICON_FA_WRENCH L"\xef\x82\xad"	// U+f0ad
#define ICON_FA_X_RAY L"\xef\x92\x97"	// U+f497
#define ICON_FA_YEN_SIGN L"\xef\x85\x97"	// U+f157
#define ICON_FA_YIN_YANG L"\xef\x9a\xad"	// U+f6ad
}
