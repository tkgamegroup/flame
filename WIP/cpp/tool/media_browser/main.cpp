int main(int argc, char **args)
{
	load_tags();
	load_pics();

	create_tags_list();
	create_grid_widgets();
	create_detail_widgets();

	return 0;
}
