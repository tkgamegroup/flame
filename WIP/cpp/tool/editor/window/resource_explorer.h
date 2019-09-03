struct ResourceExplorer : flame::ui::FileSelector
{
	ResourceExplorer();
	virtual ~ResourceExplorer() override;
	virtual void on_right_area_show() override;
};

extern ResourceExplorer *resourceExplorer;