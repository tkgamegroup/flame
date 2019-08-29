#include <flame/foundation/serialize.h>
#include <flame/foundation/bitmap.h>

#include <functional>

using namespace flame;

int main(int argc, char **args)
{
	std::vector<std::string> inputs;
	std::wstring output;
	for (auto i = 1; i < argc; i++)
	{
		if (args[i] == std::string("-o"))
		{
			i++;
			if (i < argc)
				output = s2w(args[i]);
		}
		else
			inputs.push_back(args[i]);
	}

	struct Package
	{
		Vec2i pos;
		Bitmap* b;
		std::string id;
	};
	std::vector<Package> packages;
	for (auto& i : inputs)
	{
		Package p;
		p.pos = Vec2i(-1);
		p.b = Bitmap::create_from_file(s2w(i));
		p.id = i;
		packages.push_back(p);
	}
	std::sort(packages.begin(), packages.end(), [](const Package& a, const Package& b) {
		return max(a.b->size.x(), a.b->size.y()) > max(b.b->size.x(), b.b->size.y());
	});

	auto w = 512, h = 512;
	struct Node
	{
		bool used;
		Vec2u pos;
		Vec2u size;
		std::unique_ptr<Node> right;
		std::unique_ptr<Node> bottom;
	};
	auto tree = std::make_unique<Node>();
	tree->used = false;
	tree->pos = Vec2u(0);
	tree->size.x() = w;
	tree->size.y() = h;

	std::function<Node * (Node * n, const Vec2u & size)> find_node;
	find_node = [&](Node* n, const Vec2u& size)->Node* {
		if (!n->used && n->size >= size)
		{
			n->used = true;
			n->right.reset(new Node);
			n->right->used = false;
			n->right->pos = n->pos + Vec2u(size.x(), 0);
			n->right->size = Vec2u(n->size.x() - size.x(), size.y());
			n->bottom.reset(new Node);
			n->bottom->used = false;
			n->bottom->pos = n->pos + Vec2u(0, size.y());
			n->bottom->size = Vec2u(n->size.x(), n->size.x() - size.y());
			return n;
		}
		if (!n->right || !n->bottom)
			return nullptr;
		auto n1 = find_node(n->right.get(), size);
		if (n1)
			return n1;
		auto n2 = find_node(n->bottom.get(), size);
		if (n2)
			return n2;
		return nullptr;
	};

	for (auto& p : packages)
	{
		auto n = find_node(tree.get(), p.b->size + Vec2i(2));
		if (n)
			p.pos = n->pos;
	}

	auto b = Bitmap::create(Vec2u(w, h), 4, 32);
	for (auto& e : packages)
	{
		if (e.pos >= 0)
			e.b->copy_to(b, Vec2u(0), e.b->size, Vec2u(e.pos), true);
	}


	Bitmap::save_to_file(b, output);
	std::ofstream pack_file(output + L".pack");
	for (auto& p : packages)
	{
		pack_file << p.id + " " + to_string(Vec4u(Vec2u(p.pos) + 1U, b->size)) + "\n";
		Bitmap::destroy(p.b);
	}
	pack_file.close();

	return 0;
}
