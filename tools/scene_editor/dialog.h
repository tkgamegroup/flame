#pragma once

#include "app.h"
#include "explorer_abstract.h"

struct Dialog
{
	std::string title;

	virtual ~Dialog() {}
	static void open(Dialog* dialog);
	void close();
	virtual void draw() = 0;
};

struct DialogManager
{
	std::vector<std::unique_ptr<Dialog>> dialogs;

	void update();
};

extern DialogManager dialog_manager;

struct MessageDialog : Dialog
{
	std::string message;

	static void open(const std::string& title, const std::string& message);
	void draw() override;
};

struct YesNoDialog : Dialog
{
	std::function<void(bool)> callback;

	static void open(const std::string& title, const std::function<void(bool)>& callback);
	void draw() override;
};

struct InputDialog : Dialog
{
	std::string text;
	std::function<void(bool, const std::string&)> callback;

	static void open(const std::string& title, const std::function<void(bool, const std::string&)>& callback);
	void draw() override;
};

struct SelectResourceDialog : Dialog
{
	ExplorerAbstract explorer;
	std::filesystem::path path;
	std::function<void(bool, const std::filesystem::path&)> callback;

	static void open(const std::string& title, const std::function<void(bool, const std::filesystem::path&)>& callback);
	void draw() override;
};
