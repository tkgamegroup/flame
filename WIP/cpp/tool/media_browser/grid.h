const auto grid_hori_pic_cnt = 16;
const auto grid_vert_pic_cnt = 7;
extern std::vector<Pic*> grid_pic_candidates;
extern Pic *grid_slots[grid_hori_pic_cnt * grid_vert_pic_cnt];
extern flame::UI::Layout *w_grids;
extern int grid_curr_page;
extern int grid_total_page;
extern flame::UI::Layout *w_page_ctrl;
extern flame::UI::Button *w_page_prev;
extern flame::UI::Button *w_page_next;
extern flame::UI::Button *w_page;

void create_grid_widgets();
void clear_grids();
void create_grids();
void set_grid_page_text();
