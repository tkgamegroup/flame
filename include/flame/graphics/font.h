#pragma once

#include <flame/foundation/foundation.h>
#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Image;
		struct ImageView;

		inline Vec2u get_latin_code_range()
		{
			return Vec2u(0x20, 0xff);
		}

		struct Font
		{
			virtual void release() = 0;

			virtual const wchar_t* get_filename() const = 0;

			FLAME_GRAPHICS_EXPORTS static Font* create(const wchar_t* filename);
		};

		struct Glyph
		{
			virtual ushort get_code() const = 0;
			virtual Vec2i get_off() const = 0;
			virtual Vec2u get_size() const = 0;
			virtual Vec4f get_uv() const = 0;
			virtual int get_advance() const = 0;
		};

		struct FontAtlas
		{
			virtual void release() = 0;

			inline Vec2u text_offset(uint font_size, const wchar_t* begin, const wchar_t* end = nullptr)
			{
				auto off = Vec2u(0);

				auto pstr = begin;
				while (*pstr && pstr != end)
				{
					auto ch = *pstr;
					if (ch == '\n')
					{
						off.x() = 0.f;
						off.y() += font_size;
					}
					else if (ch != '\r')
					{
						if (ch == '\t')
							ch = ' ';
						off.x() += get_glyph(ch, font_size)->get_advance();
					}
					pstr++;
				}
				return off;
			}

			inline Vec2u text_size(uint font_size, const wchar_t* begin, const wchar_t* end = nullptr)
			{
				auto size = Vec2u(0, font_size);
				auto x = 0U;

				auto pstr = begin;
				while (*pstr && pstr != end)
				{
					auto ch = *pstr;
					if (ch == '\n')
					{
						size.y() += font_size;
						x = 0;
					}
					else if (ch != '\r')
					{
						if (ch == '\t')
							ch = ' ';
						x += get_glyph(ch, font_size)->get_advance();
						size.x() = max(size.x(), x);
					}
					pstr++;
				}
				return size;
			}

			inline std::wstring wrap_text(uint font_size, uint width, const wchar_t* begin, const wchar_t* end = nullptr)
			{
				if (font_size > width)
				{
					fassert(0);
					return L"";
				}

				auto ret = std::wstring();
				auto w = 0U;

				auto pstr = begin;
				while (*pstr && pstr != end)
				{
					auto ch = *pstr;
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
						auto adv = get_glyph(ch, font_size)->get_advance();
						if (w + adv >= width)
						{
							w = adv;
							ret += '\n';
						}
						else
							w += adv;
						ret += ch;
					}
					pstr++;
				}

				return ret;
			}

			virtual Glyph* get_glyph(wchar_t unicode, uint font_size) = 0;

			virtual ImageView* get_view() const = 0;

			FLAME_GRAPHICS_EXPORTS static FontAtlas* create(Device* device, uint fonts_count, Font* const* fonts);
		};
	}

	// Font Awesome:

	const int IconMin = 0xf000;
	const int IconMax = 0xf2e0;
	const auto Icon_GLASS = L"\uf000";// 
	const auto Icon_MUSIC = L"\uf001";// 
	const auto Icon_SEARCH = L"\uf002";// 
	const auto Icon_ENVELOPE_O = L"\uf003";// 
	const auto Icon_HEART = L"\uf004";// 
	const auto Icon_STAR = L"\uf005";// 
	const auto Icon_STAR_O = L"\uf006";// 
	const auto Icon_USER = L"\uf007";// 
	const auto Icon_FILM = L"\uf008";// 
	const auto Icon_TH_LARGE = L"\uf009";// 
	const auto Icon_TH = L"\uf00a";// 
	const auto Icon_TH_LIST = L"\uf00b";// 
	const auto Icon_CHECK = L"\uf00c";// 
	const auto Icon_TIMES = L"\uf00d";// 
	const auto Icon_SEARCH_PLUS = L"\uf00e";// 
	const auto Icon_SEARCH_MINUS = L"\uf010";// 
	const auto Icon_POWER_OFF = L"\uf011";// 
	const auto Icon_SIGNAL = L"\uf012";// 
	const auto Icon_COG = L"\uf013";// 
	const auto Icon_TRASH_O = L"\uf014";// 
	const auto Icon_HOME = L"\uf015";// 
	const auto Icon_FILE_O = L"\uf016";// 
	const auto Icon_CLOCK_O = L"\uf017";// 
	const auto Icon_ROAD = L"\uf018";// 
	const auto Icon_DOWNLOAD = L"\uf019";// 
	const auto Icon_ARROW_CIRCLE_O_DOWN = L"\uf01a";// 
	const auto Icon_ARROW_CIRCLE_O_UP = L"\uf01b";// 
	const auto Icon_INBOX = L"\uf01c";// 
	const auto Icon_PLAY_CIRCLE_O = L"\uf01d";// 
	const auto Icon_REPEAT = L"\uf01e";// 
	const auto Icon_REFRESH = L"\uf021";// 
	const auto Icon_LIST_ALT = L"\uf022";// 
	const auto Icon_LOCK = L"\uf023";// 
	const auto Icon_FLAG = L"\uf024";// 
	const auto Icon_HEADPHONES = L"\uf025";// 
	const auto Icon_VOLUME_OFF = L"\uf026";// 
	const auto Icon_VOLUME_DOWN = L"\uf027";// 
	const auto Icon_VOLUME_UP = L"\uf028";// 
	const auto Icon_QRCODE = L"\uf029";// 
	const auto Icon_BARCODE = L"\uf02a";// 
	const auto Icon_TAG = L"\uf02b";// 
	const auto Icon_TAGS = L"\uf02c";// 
	const auto Icon_BOOK = L"\uf02d";// 
	const auto Icon_BOOKMARK = L"\uf02e";// 
	const auto Icon_PRINT = L"\uf02f";// 
	const auto Icon_CAMERA = L"\uf030";// 
	const auto Icon_FONT = L"\uf031";// 
	const auto Icon_BOLD = L"\uf032";// 
	const auto Icon_ITALIC = L"\uf033";// 
	const auto Icon_TEXT_HEIGHT = L"\uf034";// 
	const auto Icon_TEXT_WIDTH = L"\uf035";// 
	const auto Icon_ALIGN_LEFT = L"\uf036";// 
	const auto Icon_ALIGN_CENTER = L"\uf037";// 
	const auto Icon_ALIGN_RIGHT = L"\uf038";// 
	const auto Icon_ALIGN_JUSTIFY = L"\uf039";// 
	const auto Icon_LIST = L"\uf03a";// 
	const auto Icon_OUTDENT = L"\uf03b";// 
	const auto Icon_INDENT = L"\uf03c";// 
	const auto Icon_VIDEO_CAMERA = L"\uf03d";// 
	const auto Icon_PICTURE_O = L"\uf03e";// 
	const auto Icon_PENCIL = L"\uf040";// 
	const auto Icon_MAP_MARKER = L"\uf041";// 
	const auto Icon_ADJUST = L"\uf042";// 
	const auto Icon_TINT = L"\uf043";// 
	const auto Icon_PENCIL_SQUARE_O = L"\uf044";// 
	const auto Icon_SHARE_SQUARE_O = L"\uf045";// 
	const auto Icon_CHECK_SQUARE_O = L"\uf046";// 
	const auto Icon_ARROWS = L"\uf047";// 
	const auto Icon_STEP_BACKWARD = L"\uf048";// 
	const auto Icon_FAST_BACKWARD = L"\uf049";// 
	const auto Icon_BACKWARD = L"\uf04a";// 
	const auto Icon_PLAY = L"\uf04b";// 
	const auto Icon_PAUSE = L"\uf04c";// 
	const auto Icon_STOP = L"\uf04d";// 
	const auto Icon_FORWARD = L"\uf04e";// 
	const auto Icon_FAST_FORWARD = L"\uf050";// 
	const auto Icon_STEP_FORWARD = L"\uf051";// 
	const auto Icon_EJECT = L"\uf052";// 
	const auto Icon_CHEVRON_LEFT = L"\uf053";// 
	const auto Icon_CHEVRON_RIGHT = L"\uf054";// 
	const auto Icon_PLUS_CIRCLE = L"\uf055";// 
	const auto Icon_MINUS_CIRCLE = L"\uf056";// 
	const auto Icon_TIMES_CIRCLE = L"\uf057";// 
	const auto Icon_CHECK_CIRCLE = L"\uf058";// 
	const auto Icon_QUESTION_CIRCLE = L"\uf059";// 
	const auto Icon_INFO_CIRCLE = L"\uf05a";// 
	const auto Icon_CROSSHAIRS = L"\uf05b";// 
	const auto Icon_TIMES_CIRCLE_O = L"\uf05c";// 
	const auto Icon_CHECK_CIRCLE_O = L"\uf05d";// 
	const auto Icon_BAN = L"\uf05e";// 
	const auto Icon_ARROW_LEFT = L"\uf060";// 
	const auto Icon_ARROW_RIGHT = L"\uf061";// 
	const auto Icon_ARROW_UP = L"\uf062";// 
	const auto Icon_ARROW_DOWN = L"\uf063";// 
	const auto Icon_SHARE = L"\uf064";// 
	const auto Icon_EXPAND = L"\uf065";// 
	const auto Icon_COMPRESS = L"\uf066";// 
	const auto Icon_PLUS = L"\uf067";// 
	const auto Icon_MINUS = L"\uf068";// 
	const auto Icon_ASTERISK = L"\uf069";// 
	const auto Icon_EXCLAMATION_CIRCLE = L"\uf06a";// 
	const auto Icon_GIFT = L"\uf06b";// 
	const auto Icon_LEAF = L"\uf06c";// 
	const auto Icon_FIRE = L"\uf06d";// 
	const auto Icon_EYE = L"\uf06e";// 
	const auto Icon_EYE_SLASH = L"\uf070";// 
	const auto Icon_EXCLAMATION_TRIANGLE = L"\uf071";// 
	const auto Icon_PLANE = L"\uf072";// 
	const auto Icon_CALENDAR = L"\uf073";// 
	const auto Icon_RANDOM = L"\uf074";// 
	const auto Icon_COMMENT = L"\uf075";// 
	const auto Icon_MAGNET = L"\uf076";// 
	const auto Icon_CHEVRON_UP = L"\uf077";// 
	const auto Icon_CHEVRON_DOWN = L"\uf078";// 
	const auto Icon_RETWEET = L"\uf079";// 
	const auto Icon_SHOPPING_CART = L"\uf07a";// 
	const auto Icon_FOLDER = L"\uf07b";// 
	const auto Icon_FOLDER_OPEN = L"\uf07c";// 
	const auto Icon_ARROWS_V = L"\uf07d";// 
	const auto Icon_ARROWS_H = L"\uf07e";// 
	const auto Icon_BAR_CHART = L"\uf080";// 
	const auto Icon_TWITTER_SQUARE = L"\uf081";// 
	const auto Icon_FACEBOOK_SQUARE = L"\uf082";// 
	const auto Icon_CAMERA_RETRO = L"\uf083";// 
	const auto Icon_KEY = L"\uf084";// 
	const auto Icon_COGS = L"\uf085";// 
	const auto Icon_COMMENTS = L"\uf086";// 
	const auto Icon_THUMBS_O_UP = L"\uf087";// 
	const auto Icon_THUMBS_O_DOWN = L"\uf088";// 
	const auto Icon_STAR_HALF = L"\uf089";// 
	const auto Icon_HEART_O = L"\uf08a";// 
	const auto Icon_SIGN_OUT = L"\uf08b";// 
	const auto Icon_LINKEDIN_SQUARE = L"\uf08c";// 
	const auto Icon_THUMB_TACK = L"\uf08d";// 
	const auto Icon_EXTERNAL_LINK = L"\uf08e";// 
	const auto Icon_SIGN_IN = L"\uf090";// 
	const auto Icon_TROPHY = L"\uf091";// 
	const auto Icon_GITHUB_SQUARE = L"\uf092";// 
	const auto Icon_UPLOAD = L"\uf093";// 
	const auto Icon_LEMON_O = L"\uf094";// 
	const auto Icon_PHONE = L"\uf095";// 
	const auto Icon_SQUARE_O = L"\uf096";// 
	const auto Icon_BOOKMARK_O = L"\uf097";// 
	const auto Icon_PHONE_SQUARE = L"\uf098";// 
	const auto Icon_TWITTER = L"\uf099";// 
	const auto Icon_FACEBOOK = L"\uf09a";// 
	const auto Icon_GITHUB = L"\uf09b";// 
	const auto Icon_UNLOCK = L"\uf09c";// 
	const auto Icon_CREDIT_CARD = L"\uf09d";// 
	const auto Icon_RSS = L"\uf09e";// 
	const auto Icon_HDD_O = L"\uf0a0";// 
	const auto Icon_BULLHORN = L"\uf0a1";// 
	const auto Icon_BELL = L"\uf0f3";// 
	const auto Icon_CERTIFICATE = L"\uf0a3";// 
	const auto Icon_HAND_O_RIGHT = L"\uf0a4";// 
	const auto Icon_HAND_O_LEFT = L"\uf0a5";// 
	const auto Icon_HAND_O_UP = L"\uf0a6";// 
	const auto Icon_HAND_O_DOWN = L"\uf0a7";// 
	const auto Icon_ARROW_CIRCLE_LEFT = L"\uf0a8";// 
	const auto Icon_ARROW_CIRCLE_RIGHT = L"\uf0a9";// 
	const auto Icon_ARROW_CIRCLE_UP = L"\uf0aa";// 
	const auto Icon_ARROW_CIRCLE_DOWN = L"\uf0ab";// 
	const auto Icon_GLOBE = L"\uf0ac";// 
	const auto Icon_WRENCH = L"\uf0ad";// 
	const auto Icon_TASKS = L"\uf0ae";// 
	const auto Icon_FILTER = L"\uf0b0";// 
	const auto Icon_BRIEFCASE = L"\uf0b1";// 
	const auto Icon_ARROWS_ALT = L"\uf0b2";// 
	const auto Icon_USERS = L"\uf0c0";// 
	const auto Icon_LINK = L"\uf0c1";// 
	const auto Icon_CLOUD = L"\uf0c2";// 
	const auto Icon_FLASK = L"\uf0c3";// 
	const auto Icon_SCISSORS = L"\uf0c4";// 
	const auto Icon_FILES_O = L"\uf0c5";// 
	const auto Icon_PAPERCLIP = L"\uf0c6";// 
	const auto Icon_FLOPPY_O = L"\uf0c7";// 
	const auto Icon_SQUARE = L"\uf0c8";// 
	const auto Icon_BARS = L"\uf0c9";// 
	const auto Icon_LIST_UL = L"\uf0ca";// 
	const auto Icon_LIST_OL = L"\uf0cb";// 
	const auto Icon_STRIKETHROUGH = L"\uf0cc";// 
	const auto Icon_UNDERLINE = L"\uf0cd";// 
	const auto Icon_TABLE = L"\uf0ce";// 
	const auto Icon_MAGIC = L"\uf0d0";// 
	const auto Icon_TRUCK = L"\uf0d1";// 
	const auto Icon_PINTEREST = L"\uf0d2";// 
	const auto Icon_PINTEREST_SQUARE = L"\uf0d3";// 
	const auto Icon_GOOGLE_PLUS_SQUARE = L"\uf0d4";// 
	const auto Icon_GOOGLE_PLUS = L"\uf0d5";// 
	const auto Icon_MONEY = L"\uf0d6";// 
	const auto Icon_CARET_DOWN = L"\uf0d7";// 
	const auto Icon_CARET_UP = L"\uf0d8";// 
	const auto Icon_CARET_LEFT = L"\uf0d9";// 
	const auto Icon_CARET_RIGHT = L"\uf0da";// 
	const auto Icon_COLUMNS = L"\uf0db";// 
	const auto Icon_SORT = L"\uf0dc";// 
	const auto Icon_SORT_DESC = L"\uf0dd";// 
	const auto Icon_SORT_ASC = L"\uf0de";// 
	const auto Icon_ENVELOPE = L"\uf0e0";// 
	const auto Icon_LINKEDIN = L"\uf0e1";// 
	const auto Icon_UNDO = L"\uf0e2";// 
	const auto Icon_GAVEL = L"\uf0e3";// 
	const auto Icon_TACHOMETER = L"\uf0e4";// 
	const auto Icon_COMMENT_O = L"\uf0e5";// 
	const auto Icon_COMMENTS_O = L"\uf0e6";// 
	const auto Icon_BOLT = L"\uf0e7";// 
	const auto Icon_SITEMAP = L"\uf0e8";// 
	const auto Icon_UMBRELLA = L"\uf0e9";// 
	const auto Icon_CLIPBOARD = L"\uf0ea";// 
	const auto Icon_LIGHTBULB_O = L"\uf0eb";// 
	const auto Icon_EXCHANGE = L"\uf0ec";// 
	const auto Icon_CLOUD_DOWNLOAD = L"\uf0ed";// 
	const auto Icon_CLOUD_UPLOAD = L"\uf0ee";// 
	const auto Icon_USER_MD = L"\uf0f0";// 
	const auto Icon_STETHOSCOPE = L"\uf0f1";// 
	const auto Icon_SUITCASE = L"\uf0f2";// 
	const auto Icon_BELL_O = L"\uf0a2";// 
	const auto Icon_COFFEE = L"\uf0f4";// 
	const auto Icon_CUTLERY = L"\uf0f5";// 
	const auto Icon_FILE_TEXT_O = L"\uf0f6";// 
	const auto Icon_BUILDING_O = L"\uf0f7";// 
	const auto Icon_HOSPITAL_O = L"\uf0f8";// 
	const auto Icon_AMBULANCE = L"\uf0f9";// 
	const auto Icon_MEDKIT = L"\uf0fa";// 
	const auto Icon_FIGHTER_JET = L"\uf0fb";// 
	const auto Icon_BEER = L"\uf0fc";// 
	const auto Icon_H_SQUARE = L"\uf0fd";// 
	const auto Icon_PLUS_SQUARE = L"\uf0fe";// 
	const auto Icon_ANGLE_DOUBLE_LEFT = L"\uf100";// 
	const auto Icon_ANGLE_DOUBLE_RIGHT = L"\uf101";// 
	const auto Icon_ANGLE_DOUBLE_UP = L"\uf102";// 
	const auto Icon_ANGLE_DOUBLE_DOWN = L"\uf103";// 
	const auto Icon_ANGLE_LEFT = L"\uf104";// 
	const auto Icon_ANGLE_RIGHT = L"\uf105";// 
	const auto Icon_ANGLE_UP = L"\uf106";// 
	const auto Icon_ANGLE_DOWN = L"\uf107";// 
	const auto Icon_DESKTOP = L"\uf108";// 
	const auto Icon_LAPTOP = L"\uf109";// 
	const auto Icon_TABLET = L"\uf10a";// 
	const auto Icon_MOBILE = L"\uf10b";// 
	const auto Icon_CIRCLE_O = L"\uf10c";// 
	const auto Icon_QUOTE_LEFT = L"\uf10d";// 
	const auto Icon_QUOTE_RIGHT = L"\uf10e";// 
	const auto Icon_SPINNER = L"\uf110";// 
	const auto Icon_CIRCLE = L"\uf111";// 
	const auto Icon_REPLY = L"\uf112";// 
	const auto Icon_GITHUB_ALT = L"\uf113";// 
	const auto Icon_FOLDER_O = L"\uf114";// 
	const auto Icon_FOLDER_OPEN_O = L"\uf115";// 
	const auto Icon_SMILE_O = L"\uf118";// 
	const auto Icon_FROWN_O = L"\uf119";// 
	const auto Icon_MEH_O = L"\uf11a";// 
	const auto Icon_GAMEPAD = L"\uf11b";// 
	const auto Icon_KEYBOARD_O = L"\uf11c";// 
	const auto Icon_FLAG_O = L"\uf11d";// 
	const auto Icon_FLAG_CHECKERED = L"\uf11e";// 
	const auto Icon_TERMINAL = L"\uf120";// 
	const auto Icon_CODE = L"\uf121";// 
	const auto Icon_REPLY_ALL = L"\uf122";// 
	const auto Icon_STAR_HALF_O = L"\uf123";// 
	const auto Icon_LOCATION_ARROW = L"\uf124";// 
	const auto Icon_CROP = L"\uf125";// 
	const auto Icon_CODE_FORK = L"\uf126";// 
	const auto Icon_CHAIN_BROKEN = L"\uf127";// 
	const auto Icon_QUESTION = L"\uf128";// 
	const auto Icon_INFO = L"\uf129";// 
	const auto Icon_EXCLAMATION = L"\uf12a";// 
	const auto Icon_SUPERSCRIPT = L"\uf12b";// 
	const auto Icon_SUBSCRIPT = L"\uf12c";// 
	const auto Icon_ERASER = L"\uf12d";// 
	const auto Icon_PUZZLE_PIECE = L"\uf12e";// 
	const auto Icon_MICROPHONE = L"\uf130";// 
	const auto Icon_MICROPHONE_SLASH = L"\uf131";// 
	const auto Icon_SHIELD = L"\uf132";// 
	const auto Icon_CALENDAR_O = L"\uf133";// 
	const auto Icon_FIRE_EXTINGUISHER = L"\uf134";// 
	const auto Icon_ROCKET = L"\uf135";// 
	const auto Icon_MAXCDN = L"\uf136";// 
	const auto Icon_CHEVRON_CIRCLE_LEFT = L"\uf137";// 
	const auto Icon_CHEVRON_CIRCLE_RIGHT = L"\uf138";// 
	const auto Icon_CHEVRON_CIRCLE_UP = L"\uf139";// 
	const auto Icon_CHEVRON_CIRCLE_DOWN = L"\uf13a";// 
	const auto Icon_HTML5 = L"\uf13b";// 
	const auto Icon_CSS3 = L"\uf13c";// 
	const auto Icon_ANCHOR = L"\uf13d";// 
	const auto Icon_UNLOCK_ALT = L"\uf13e";// 
	const auto Icon_BULLSEYE = L"\uf140";// 
	const auto Icon_ELLIPSIS_H = L"\uf141";// 
	const auto Icon_ELLIPSIS_V = L"\uf142";// 
	const auto Icon_RSS_SQUARE = L"\uf143";// 
	const auto Icon_PLAY_CIRCLE = L"\uf144";// 
	const auto Icon_TICKET = L"\uf145";// 
	const auto Icon_MINUS_SQUARE = L"\uf146";// 
	const auto Icon_MINUS_SQUARE_O = L"\uf147";// 
	const auto Icon_LEVEL_UP = L"\uf148";// 
	const auto Icon_LEVEL_DOWN = L"\uf149";// 
	const auto Icon_CHECK_SQUARE = L"\uf14a";// 
	const auto Icon_PENCIL_SQUARE = L"\uf14b";// 
	const auto Icon_EXTERNAL_LINK_SQUARE = L"\uf14c";// 
	const auto Icon_SHARE_SQUARE = L"\uf14d";// 
	const auto Icon_COMPASS = L"\uf14e";// 
	const auto Icon_CARET_SQUARE_O_DOWN = L"\uf150";// 
	const auto Icon_CARET_SQUARE_O_UP = L"\uf151";// 
	const auto Icon_CARET_SQUARE_O_RIGHT = L"\uf152";// 
	const auto Icon_EUR = L"\uf153";// 
	const auto Icon_GBP = L"\uf154";// 
	const auto Icon_USD = L"\uf155";// 
	const auto Icon_INR = L"\uf156";// 
	const auto Icon_JPY = L"\uf157";// 
	const auto Icon_RUB = L"\uf158";// 
	const auto Icon_KRW = L"\uf159";// 
	const auto Icon_BTC = L"\uf15a";// 
	const auto Icon_FILE = L"\uf15b";// 
	const auto Icon_FILE_TEXT = L"\uf15c";// 
	const auto Icon_SORT_ALPHA_ASC = L"\uf15d";// 
	const auto Icon_SORT_ALPHA_DESC = L"\uf15e";// 
	const auto Icon_SORT_AMOUNT_ASC = L"\uf160";// 
	const auto Icon_SORT_AMOUNT_DESC = L"\uf161";// 
	const auto Icon_SORT_NUMERIC_ASC = L"\uf162";// 
	const auto Icon_SORT_NUMERIC_DESC = L"\uf163";// 
	const auto Icon_THUMBS_UP = L"\uf164";// 
	const auto Icon_THUMBS_DOWN = L"\uf165";// 
	const auto Icon_YOUTUBE_SQUARE = L"\uf166";// 
	const auto Icon_YOUTUBE = L"\uf167";// 
	const auto Icon_XING = L"\uf168";// 
	const auto Icon_XING_SQUARE = L"\uf169";// 
	const auto Icon_YOUTUBE_PLAY = L"\uf16a";// 
	const auto Icon_DROPBOX = L"\uf16b";// 
	const auto Icon_STACK_OVERFLOW = L"\uf16c";// 
	const auto Icon_INSTAGRAM = L"\uf16d";// 
	const auto Icon_FLICKR = L"\uf16e";// 
	const auto Icon_ADN = L"\uf170";// 
	const auto Icon_BITBUCKET = L"\uf171";// 
	const auto Icon_BITBUCKET_SQUARE = L"\uf172";// 
	const auto Icon_TUMBLR = L"\uf173";// 
	const auto Icon_TUMBLR_SQUARE = L"\uf174";// 
	const auto Icon_LONG_ARROW_DOWN = L"\uf175";// 
	const auto Icon_LONG_ARROW_UP = L"\uf176";// 
	const auto Icon_LONG_ARROW_LEFT = L"\uf177";// 
	const auto Icon_LONG_ARROW_RIGHT = L"\uf178";// 
	const auto Icon_APPLE = L"\uf179";// 
	const auto Icon_WINDOWS = L"\uf17a";// 
	const auto Icon_ANDROID = L"\uf17b";// 
	const auto Icon_LINUX = L"\uf17c";// 
	const auto Icon_DRIBBBLE = L"\uf17d";// 
	const auto Icon_SKYPE = L"\uf17e";// 
	const auto Icon_FOURSQUARE = L"\uf180";// 
	const auto Icon_TRELLO = L"\uf181";// 
	const auto Icon_FEMALE = L"\uf182";// 
	const auto Icon_MALE = L"\uf183";// 
	const auto Icon_GRATIPAY = L"\uf184";// 
	const auto Icon_SUN_O = L"\uf185";// 
	const auto Icon_MOON_O = L"\uf186";// 
	const auto Icon_ARCHIVE = L"\uf187";// 
	const auto Icon_BUG = L"\uf188";// 
	const auto Icon_VK = L"\uf189";// 
	const auto Icon_WEIBO = L"\uf18a";// 
	const auto Icon_RENREN = L"\uf18b";// 
	const auto Icon_PAGELINES = L"\uf18c";// 
	const auto Icon_STACK_EXCHANGE = L"\uf18d";// 
	const auto Icon_ARROW_CIRCLE_O_RIGHT = L"\uf18e";// 
	const auto Icon_ARROW_CIRCLE_O_LEFT = L"\uf190";// 
	const auto Icon_CARET_SQUARE_O_LEFT = L"\uf191";// 
	const auto Icon_DOT_CIRCLE_O = L"\uf192";// 
	const auto Icon_WHEELCHAIR = L"\uf193";// 
	const auto Icon_VIMEO_SQUARE = L"\uf194";// 
	const auto Icon_TRY = L"\uf195";// 
	const auto Icon_PLUS_SQUARE_O = L"\uf196";// 
	const auto Icon_SPACE_SHUTTLE = L"\uf197";// 
	const auto Icon_SLACK = L"\uf198";// 
	const auto Icon_ENVELOPE_SQUARE = L"\uf199";// 
	const auto Icon_WORDPRESS = L"\uf19a";// 
	const auto Icon_OPENID = L"\uf19b";// 
	const auto Icon_UNIVERSITY = L"\uf19c";// 
	const auto Icon_GRADUATION_CAP = L"\uf19d";// 
	const auto Icon_YAHOO = L"\uf19e";// 
	const auto Icon_GOOGLE = L"\uf1a0";// 
	const auto Icon_REDDIT = L"\uf1a1";// 
	const auto Icon_REDDIT_SQUARE = L"\uf1a2";// 
	const auto Icon_STUMBLEUPON_CIRCLE = L"\uf1a3";// 
	const auto Icon_STUMBLEUPON = L"\uf1a4";// 
	const auto Icon_DELICIOUS = L"\uf1a5";// 
	const auto Icon_DIGG = L"\uf1a6";// 
	const auto Icon_PIED_PIPER_PP = L"\uf1a7";// 
	const auto Icon_PIED_PIPER_ALT = L"\uf1a8";// 
	const auto Icon_DRUPAL = L"\uf1a9";// 
	const auto Icon_JOOMLA = L"\uf1aa";// 
	const auto Icon_LANGUAGE = L"\uf1ab";// 
	const auto Icon_FAX = L"\uf1ac";// 
	const auto Icon_BUILDING = L"\uf1ad";// 
	const auto Icon_CHILD = L"\uf1ae";// 
	const auto Icon_PAW = L"\uf1b0";// 
	const auto Icon_SPOON = L"\uf1b1";// 
	const auto Icon_CUBE = L"\uf1b2";// 
	const auto Icon_CUBES = L"\uf1b3";// 
	const auto Icon_BEHANCE = L"\uf1b4";// 
	const auto Icon_BEHANCE_SQUARE = L"\uf1b5";// 
	const auto Icon_STEAM = L"\uf1b6";// 
	const auto Icon_STEAM_SQUARE = L"\uf1b7";// 
	const auto Icon_RECYCLE = L"\uf1b8";// 
	const auto Icon_CAR = L"\uf1b9";// 
	const auto Icon_TAXI = L"\uf1ba";// 
	const auto Icon_TREE = L"\uf1bb";// 
	const auto Icon_SPOTIFY = L"\uf1bc";// 
	const auto Icon_DEVIANTART = L"\uf1bd";// 
	const auto Icon_SOUNDCLOUD = L"\uf1be";// 
	const auto Icon_DATABASE = L"\uf1c0";// 
	const auto Icon_FILE_PDF_O = L"\uf1c1";// 
	const auto Icon_FILE_WORD_O = L"\uf1c2";// 
	const auto Icon_FILE_EXCEL_O = L"\uf1c3";// 
	const auto Icon_FILE_POWERPOINT_O = L"\uf1c4";// 
	const auto Icon_FILE_IMAGE_O = L"\uf1c5";// 
	const auto Icon_FILE_ARCHIVE_O = L"\uf1c6";// 
	const auto Icon_FILE_AUDIO_O = L"\uf1c7";// 
	const auto Icon_FILE_VIDEO_O = L"\uf1c8";// 
	const auto Icon_FILE_CODE_O = L"\uf1c9";// 
	const auto Icon_VINE = L"\uf1ca";// 
	const auto Icon_CODEPEN = L"\uf1cb";// 
	const auto Icon_JSFIDDLE = L"\uf1cc";// 
	const auto Icon_LIFE_RING = L"\uf1cd";// 
	const auto Icon_CIRCLE_O_NOTCH = L"\uf1ce";// 
	const auto Icon_REBEL = L"\uf1d0";// 
	const auto Icon_EMPIRE = L"\uf1d1";// 
	const auto Icon_GIT_SQUARE = L"\uf1d2";// 
	const auto Icon_GIT = L"\uf1d3";// 
	const auto Icon_HACKER_NEWS = L"\uf1d4";// 
	const auto Icon_TENCENT_WEIBO = L"\uf1d5";// 
	const auto Icon_QQ = L"\uf1d6";// 
	const auto Icon_WEIXIN = L"\uf1d7";// 
	const auto Icon_PAPER_PLANE = L"\uf1d8";// 
	const auto Icon_PAPER_PLANE_O = L"\uf1d9";// 
	const auto Icon_HISTORY = L"\uf1da";// 
	const auto Icon_CIRCLE_THIN = L"\uf1db";// 
	const auto Icon_HEADER = L"\uf1dc";// 
	const auto Icon_PARAGRAPH = L"\uf1dd";// 
	const auto Icon_SLIDERS = L"\uf1de";// 
	const auto Icon_SHARE_ALT = L"\uf1e0";// 
	const auto Icon_SHARE_ALT_SQUARE = L"\uf1e1";// 
	const auto Icon_BOMB = L"\uf1e2";// 
	const auto Icon_FUTBOL_O = L"\uf1e3";// 
	const auto Icon_TTY = L"\uf1e4";// 
	const auto Icon_BINOCULARS = L"\uf1e5";// 
	const auto Icon_PLUG = L"\uf1e6";// 
	const auto Icon_SLIDESHARE = L"\uf1e7";// 
	const auto Icon_TWITCH = L"\uf1e8";// 
	const auto Icon_YELP = L"\uf1e9";// 
	const auto Icon_NEWSPAPER_O = L"\uf1ea";// 
	const auto Icon_WIFI = L"\uf1eb";// 
	const auto Icon_CALCULATOR = L"\uf1ec";// 
	const auto Icon_PAYPAL = L"\uf1ed";// 
	const auto Icon_GOOGLE_WALLET = L"\uf1ee";// 
	const auto Icon_CC_VISA = L"\uf1f0";// 
	const auto Icon_CC_MASTERCARD = L"\uf1f1";// 
	const auto Icon_CC_DISCOVER = L"\uf1f2";// 
	const auto Icon_CC_AMEX = L"\uf1f3";// 
	const auto Icon_CC_PAYPAL = L"\uf1f4";// 
	const auto Icon_CC_STRIPE = L"\uf1f5";// 
	const auto Icon_BELL_SLASH = L"\uf1f6";// 
	const auto Icon_BELL_SLASH_O = L"\uf1f7";// 
	const auto Icon_TRASH = L"\uf1f8";// 
	const auto Icon_COPYRIGHT = L"\uf1f9";// 
	const auto Icon_AT = L"\uf1fa";// 
	const auto Icon_EYEDROPPER = L"\uf1fb";// 
	const auto Icon_PAINT_BRUSH = L"\uf1fc";// 
	const auto Icon_BIRTHDAY_CAKE = L"\uf1fd";// 
	const auto Icon_AREA_CHART = L"\uf1fe";// 
	const auto Icon_PIE_CHART = L"\uf200";// 
	const auto Icon_LINE_CHART = L"\uf201";// 
	const auto Icon_LASTFM = L"\uf202";// 
	const auto Icon_LASTFM_SQUARE = L"\uf203";// 
	const auto Icon_TOGGLE_OFF = L"\uf204";// 
	const auto Icon_TOGGLE_ON = L"\uf205";// 
	const auto Icon_BICYCLE = L"\uf206";// 
	const auto Icon_BUS = L"\uf207";// 
	const auto Icon_IOXHOST = L"\uf208";// 
	const auto Icon_ANGELLIST = L"\uf209";// 
	const auto Icon_CC = L"\uf20a";// 
	const auto Icon_ILS = L"\uf20b";// 
	const auto Icon_MEANPATH = L"\uf20c";// 
	const auto Icon_BUYSELLADS = L"\uf20d";// 
	const auto Icon_CONNECTDEVELOP = L"\uf20e";// 
	const auto Icon_DASHCUBE = L"\uf210";// 
	const auto Icon_FORUMBEE = L"\uf211";// 
	const auto Icon_LEANPUB = L"\uf212";// 
	const auto Icon_SELLSY = L"\uf213";// 
	const auto Icon_SHIRTSINBULK = L"\uf214";// 
	const auto Icon_SIMPLYBUILT = L"\uf215";// 
	const auto Icon_SKYATLAS = L"\uf216";// 
	const auto Icon_CART_PLUS = L"\uf217";// 
	const auto Icon_CART_ARROW_DOWN = L"\uf218";// 
	const auto Icon_DIAMOND = L"\uf219";// 
	const auto Icon_SHIP = L"\uf21a";// 
	const auto Icon_USER_SECRET = L"\uf21b";// 
	const auto Icon_MOTORCYCLE = L"\uf21c";// 
	const auto Icon_STREET_VIEW = L"\uf21d";// 
	const auto Icon_HEARTBEAT = L"\uf21e";// 
	const auto Icon_VENUS = L"\uf221";// 
	const auto Icon_MARS = L"\uf222";// 
	const auto Icon_MERCURY = L"\uf223";// 
	const auto Icon_TRANSGENDER = L"\uf224";// 
	const auto Icon_TRANSGENDER_ALT = L"\uf225";// 
	const auto Icon_VENUS_DOUBLE = L"\uf226";// 
	const auto Icon_MARS_DOUBLE = L"\uf227";// 
	const auto Icon_VENUS_MARS = L"\uf228";// 
	const auto Icon_MARS_STROKE = L"\uf229";// 
	const auto Icon_MARS_STROKE_V = L"\uf22a";// 
	const auto Icon_MARS_STROKE_H = L"\uf22b";// 
	const auto Icon_NEUTER = L"\uf22c";// 
	const auto Icon_GENDERLESS = L"\uf22d";// 
	const auto Icon_FACEBOOK_OFFICIAL = L"\uf230";// 
	const auto Icon_PINTEREST_P = L"\uf231";// 
	const auto Icon_WHATSAPP = L"\uf232";// 
	const auto Icon_SERVER = L"\uf233";// 
	const auto Icon_USER_PLUS = L"\uf234";// 
	const auto Icon_USER_TIMES = L"\uf235";// 
	const auto Icon_BED = L"\uf236";// 
	const auto Icon_VIACOIN = L"\uf237";// 
	const auto Icon_TRAIN = L"\uf238";// 
	const auto Icon_SUBWAY = L"\uf239";// 
	const auto Icon_MEDIUM = L"\uf23a";// 
	const auto Icon_Y_COMBINATOR = L"\uf23b";// 
	const auto Icon_OPTIN_MONSTER = L"\uf23c";// 
	const auto Icon_OPENCART = L"\uf23d";// 
	const auto Icon_EXPEDITEDSSL = L"\uf23e";// 
	const auto Icon_BATTERY_FULL = L"\uf240";// 
	const auto Icon_BATTERY_THREE_QUARTERS = L"\uf241";// 
	const auto Icon_BATTERY_HALF = L"\uf242";// 
	const auto Icon_BATTERY_QUARTER = L"\uf243";// 
	const auto Icon_BATTERY_EMPTY = L"\uf244";// 
	const auto Icon_MOUSE_POINTER = L"\uf245";// 
	const auto Icon_I_CURSOR = L"\uf246";// 
	const auto Icon_OBJECT_GROUP = L"\uf247";// 
	const auto Icon_OBJECT_UNGROUP = L"\uf248";// 
	const auto Icon_STICKY_NOTE = L"\uf249";// 
	const auto Icon_STICKY_NOTE_O = L"\uf24a";// 
	const auto Icon_CC_JCB = L"\uf24b";// 
	const auto Icon_CC_DINERS_CLUB = L"\uf24c";// 
	const auto Icon_CLONE = L"\uf24d";// 
	const auto Icon_BALANCE_SCALE = L"\uf24e";// 
	const auto Icon_HOURGLASS_O = L"\uf250";// 
	const auto Icon_HOURGLASS_START = L"\uf251";// 
	const auto Icon_HOURGLASS_HALF = L"\uf252";// 
	const auto Icon_HOURGLASS_END = L"\uf253";// 
	const auto Icon_HOURGLASS = L"\uf254";// 
	const auto Icon_HAND_ROCK_O = L"\uf255";// 
	const auto Icon_HAND_PAPER_O = L"\uf256";// 
	const auto Icon_HAND_SCISSORS_O = L"\uf257";// 
	const auto Icon_HAND_LIZARD_O = L"\uf258";// 
	const auto Icon_HAND_SPOCK_O = L"\uf259";// 
	const auto Icon_HAND_POINTER_O = L"\uf25a";// 
	const auto Icon_HAND_PEACE_O = L"\uf25b";// 
	const auto Icon_TRADEMARK = L"\uf25c";// 
	const auto Icon_REGISTERED = L"\uf25d";// 
	const auto Icon_CREATIVE_COMMONS = L"\uf25e";// 
	const auto Icon_GG = L"\uf260";// 
	const auto Icon_GG_CIRCLE = L"\uf261";// 
	const auto Icon_TRIPADVISOR = L"\uf262";// 
	const auto Icon_ODNOKLASSNIKI = L"\uf263";// 
	const auto Icon_ODNOKLASSNIKI_SQUARE = L"\uf264";// 
	const auto Icon_GET_POCKET = L"\uf265";// 
	const auto Icon_WIKIPEDIA_W = L"\uf266";// 
	const auto Icon_SAFARI = L"\uf267";// 
	const auto Icon_CHROME = L"\uf268";// 
	const auto Icon_FIREFOX = L"\uf269";// 
	const auto Icon_OPERA = L"\uf26a";// 
	const auto Icon_INTERNET_EXPLORER = L"\uf26b";// 
	const auto Icon_TELEVISION = L"\uf26c";// 
	const auto Icon_CONTAO = L"\uf26d";// 
	const auto Icon_500PX = L"\uf26e";// 
	const auto Icon_AMAZON = L"\uf270";// 
	const auto Icon_CALENDAR_PLUS_O = L"\uf271";// 
	const auto Icon_CALENDAR_MINUS_O = L"\uf272";// 
	const auto Icon_CALENDAR_TIMES_O = L"\uf273";// 
	const auto Icon_CALENDAR_CHECK_O = L"\uf274";// 
	const auto Icon_INDUSTRY = L"\uf275";// 
	const auto Icon_MAP_PIN = L"\uf276";// 
	const auto Icon_MAP_SIGNS = L"\uf277";// 
	const auto Icon_MAP_O = L"\uf278";// 
	const auto Icon_MAP = L"\uf279";// 
	const auto Icon_COMMENTING = L"\uf27a";// 
	const auto Icon_COMMENTING_O = L"\uf27b";// 
	const auto Icon_HOUZZ = L"\uf27c";// 
	const auto Icon_VIMEO = L"\uf27d";// 
	const auto Icon_BLACK_TIE = L"\uf27e";// 
	const auto Icon_FONTICONS = L"\uf280";// 
	const auto Icon_REDDIT_ALIEN = L"\uf281";// 
	const auto Icon_EDGE = L"\uf282";// 
	const auto Icon_CREDIT_CARD_ALT = L"\uf283";// 
	const auto Icon_CODIEPIE = L"\uf284";// 
	const auto Icon_MODX = L"\uf285";// 
	const auto Icon_FORT_AWESOME = L"\uf286";// 
	const auto Icon_USB = L"\uf287";// 
	const auto Icon_PRODUCT_HUNT = L"\uf288";// 
	const auto Icon_MIXCLOUD = L"\uf289";// 
	const auto Icon_SCRIBD = L"\uf28a";// 
	const auto Icon_PAUSE_CIRCLE = L"\uf28b";// 
	const auto Icon_PAUSE_CIRCLE_O = L"\uf28c";// 
	const auto Icon_STOP_CIRCLE = L"\uf28d";// 
	const auto Icon_STOP_CIRCLE_O = L"\uf28e";// 
	const auto Icon_SHOPPING_BAG = L"\uf290";// 
	const auto Icon_SHOPPING_BASKET = L"\uf291";// 
	const auto Icon_HASHTAG = L"\uf292";// 
	const auto Icon_BLUETOOTH = L"\uf293";// 
	const auto Icon_BLUETOOTH_B = L"\uf294";// 
	const auto Icon_PERCENT = L"\uf295";// 
	const auto Icon_GITLAB = L"\uf296";// 
	const auto Icon_WPBEGINNER = L"\uf297";// 
	const auto Icon_WPFORMS = L"\uf298";// 
	const auto Icon_ENVIRA = L"\uf299";// 
	const auto Icon_UNIVERSAL_ACCESS = L"\uf29a";// 
	const auto Icon_WHEELCHAIR_ALT = L"\uf29b";// 
	const auto Icon_QUESTION_CIRCLE_O = L"\uf29c";// 
	const auto Icon_BLIND = L"\uf29d";// 
	const auto Icon_AUDIO_DESCRIPTION = L"\uf29e";// 
	const auto Icon_VOLUME_CONTROL_PHONE = L"\uf2a0";// 
	const auto Icon_BRAILLE = L"\uf2a1";// 
	const auto Icon_ASSISTIVE_LISTENING_SYSTEMS = L"\uf2a2";// 
	const auto Icon_AMERICAN_SIGN_LANGUAGE_INTERPRETING = L"\uf2a3";// 
	const auto Icon_DEAF = L"\uf2a4";// 
	const auto Icon_GLIDE = L"\uf2a5";// 
	const auto Icon_GLIDE_G = L"\uf2a6";// 
	const auto Icon_SIGN_LANGUAGE = L"\uf2a7";// 
	const auto Icon_LOW_VISION = L"\uf2a8";// 
	const auto Icon_VIADEO = L"\uf2a9";// 
	const auto Icon_VIADEO_SQUARE = L"\uf2aa";// 
	const auto Icon_SNAPCHAT = L"\uf2ab";// 
	const auto Icon_SNAPCHAT_GHOST = L"\uf2ac";// 
	const auto Icon_SNAPCHAT_SQUARE = L"\uf2ad";// 
	const auto Icon_PIED_PIPER = L"\uf2ae";// 
	const auto Icon_FIRST_ORDER = L"\uf2b0";// 
	const auto Icon_YOAST = L"\uf2b1";// 
	const auto Icon_THEMEISLE = L"\uf2b2";// 
	const auto Icon_GOOGLE_PLUS_OFFICIAL = L"\uf2b3";// 
	const auto Icon_FONT_AWESOME = L"\uf2b4";// 
	const auto Icon_HANDSHAKE_O = L"\uf2b5";// 
	const auto Icon_ENVELOPE_OPEN = L"\uf2b6";// 
	const auto Icon_ENVELOPE_OPEN_O = L"\uf2b7";// 
	const auto Icon_LINODE = L"\uf2b8";// 
	const auto Icon_ADDRESS_BOOK = L"\uf2b9";// 
	const auto Icon_ADDRESS_BOOK_O = L"\uf2ba";// 
	const auto Icon_ADDRESS_CARD = L"\uf2bb";// 
	const auto Icon_ADDRESS_CARD_O = L"\uf2bc";// 
	const auto Icon_USER_CIRCLE = L"\uf2bd";// 
	const auto Icon_USER_CIRCLE_O = L"\uf2be";// 
	const auto Icon_USER_O = L"\uf2c0";// 
	const auto Icon_ID_BADGE = L"\uf2c1";// 
	const auto Icon_ID_CARD = L"\uf2c2";// 
	const auto Icon_ID_CARD_O = L"\uf2c3";// 
	const auto Icon_QUORA = L"\uf2c4";// 
	const auto Icon_FREE_CODE_CAMP = L"\uf2c5";// 
	const auto Icon_TELEGRAM = L"\uf2c6";// 
	const auto Icon_THERMOMETER_FULL = L"\uf2c7";// 
	const auto Icon_THERMOMETER_THREE_QUARTERS = L"\uf2c8";// 
	const auto Icon_THERMOMETER_HALF = L"\uf2c9";// 
	const auto Icon_THERMOMETER_QUARTER = L"\uf2ca";// 
	const auto Icon_THERMOMETER_EMPTY = L"\uf2cb";// 
	const auto Icon_SHOWER = L"\uf2cc";// 
	const auto Icon_BATH = L"\uf2cd";// 
	const auto Icon_PODCAST = L"\uf2ce";// 
	const auto Icon_WINDOW_MAXIMIZE = L"\uf2d0";// 
	const auto Icon_WINDOW_MINIMIZE = L"\uf2d1";// 
	const auto Icon_WINDOW_RESTORE = L"\uf2d2";// 
	const auto Icon_WINDOW_CLOSE = L"\uf2d3";// 
	const auto Icon_WINDOW_CLOSE_O = L"\uf2d4";// 
	const auto Icon_BANDCAMP = L"\uf2d5";// 
	const auto Icon_GRAV = L"\uf2d6";// 
	const auto Icon_ETSY = L"\uf2d7";// 
	const auto Icon_IMDB = L"\uf2d8";// 
	const auto Icon_RAVELRY = L"\uf2d9";// 
	const auto Icon_EERCAST = L"\uf2da";// 
	const auto Icon_MICROCHIP = L"\uf2db";// 
	const auto Icon_SNOWFLAKE_O = L"\uf2dc";// 
	const auto Icon_SUPERPOWERS = L"\uf2dd";// 
	const auto Icon_WPEXPLORER = L"\uf2de";// 
	const auto Icon_MEETUP = L"\uf2e0";// 
}
