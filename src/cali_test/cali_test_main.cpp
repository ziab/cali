#include <gtest.h>
#include <TerrainQuadTree.h>

TEST(TerrainQuadTree, quad)
{
	Cali::TerrainQuadTree::Quad quad{ { 0.5, 0.5 }, { 0.5, 0.5 } };

	ASSERT_TRUE(quad.contains({ 0.5, 0.5 }));
	ASSERT_TRUE(quad.contains({ 0.99, 0.99 }));
	ASSERT_FALSE(quad.contains({ 1.99, 1.99 }));
}

TEST(TerrainQuadTree, test_building_tree)
{
	Cali::TerrainQuadTree tqtree({ { 0.5, 0.5 }, { 0.5, 0.5} });

	ASSERT_TRUE(tqtree.divide({ 0.75, 0.75 }, 1));
	ASSERT_TRUE(tqtree.divide({ 0.25, 0.25 }, 2));
	ASSERT_FALSE(tqtree.divide({ -0.25, 0.25 }, 2));

	tqtree.get_node_at({ 0.75, 0.75 });

	std::vector<const Cali::TerrainQuadTree::Node*> nodes;
	tqtree.query_nodes(nodes);
	ASSERT_TRUE(nodes.size() == 7);

	ASSERT_TRUE(tqtree.divide({ 0.76, 0.76 }, 3));

	nodes.clear();
	tqtree.query_nodes(nodes);
	ASSERT_TRUE(nodes.size() == 13);
}

void main(int argc, char** argv)
{
	try
	{
		testing::InitGoogleTest(&argc, argv);
		auto ret = RUN_ALL_TESTS();
	}
	catch (std::exception& e)
	{
		std::cout << "exception: " << e.what() << std::endl;
	}

	system("pause");
}