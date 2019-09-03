struct Tag
{
	int id;
	std::wstring name;
	bool use;

	std::vector<Pic*> pics;

	void remove_pic(Pic *p);
};

void delete_tag(Tag *t);

extern std::vector<std::unique_ptr<Tag>> tags;

void sort_tags();
Tag *get_tag(int id);
Tag *find_tag(const std::wstring &name);
Tag *add_tag(const std::wstring &name, int id);
Tag *add_tag(const std::wstring &name);
void load_tags();
void save_tags();
std::vector<std::wstring> guess_tags_from_filename(const std::wstring &filename);
