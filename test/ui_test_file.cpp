
//{
//	auto file = SerializableNode::create("ui");

//	auto n = w_combo->save();
//	file->add_node(n);

//	file->save_xml(L"d:/ui.xml");
//	SerializableNode::destroy(file);
//}
//{
//	auto file = SerializableNode::create_from_xml(L"d:/ui.xml");
//	if (file)
//	{
//		auto w = Element::create_from_file(ui, file->node(0));

//		SerializableNode::destroy(file);

//		layout1->add_child(w);
//	}
//}
