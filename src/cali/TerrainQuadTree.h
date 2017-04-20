#pragma once
#include <vector>
#include <exception>
#include <assert.h>

namespace Cali
{
	class TerrainQuadTree
	{
	public:
		struct Point
		{
			double x, y;
			Point(double _x, double _y) : x(_x), y(_y) {}

			Point operator+(const Point& rhv) const
			{
				return Point{ x + rhv.x, y + rhv.y };
			}

			Point operator-(const Point& rhv) const
			{
				return Point{ x - rhv.x, y - rhv.y };
			}

			Point operator/(double rhv) const
			{
				return Point{ x / rhv, y / rhv };
			}

			Point& operator=(const Point& rhv)
			{
				x = rhv.x;
				y = rhv.y;
			}
		};

		struct Quad
		{
			Point center;
			Point half_size;
			Quad(Point _center, Point _half_size) :
				center(_center), half_size(_half_size) {}

			bool contains(Point a) const
			{
				if (a.x < center.x + half_size.x && a.x > center.x - half_size.x)
				{
					if (a.y < center.y + half_size.y && a.y > center.y - half_size.y)
					{
						return true;
					}
				}
				return false;
			}

			Quad& operator=(const Quad& rhv)
			{
				center = rhv.center;
				half_size = rhv.half_size;
			}

		};

		class Node
		{
		private:
			Quad m_quad;
			size_t m_depth;
			Node* m_tl;
			Node* m_tr;
			Node* m_bl;
			Node* m_br;
			Node* m_parent;

		private:
			void release_child_node(Node*& child)
			{
				if (child)
				{
					child->collapse();
					delete child;
					child = nullptr;
				}
			}

			bool has_child() const
			{
				if (m_tl && m_tr && m_bl && m_br) return true;
				assert(!(m_tl || m_tr || m_bl || m_br));
				return false;
			}

		public:
			Node() :
				m_quad({ 0.5, 0.5 }, { 0.5, 0.5 }),
				m_depth(0),
				m_tl(nullptr),
				m_tr(nullptr),
				m_bl(nullptr),
				m_br(nullptr),
				m_parent(nullptr)
			{
			}

			Node(Quad _quad, size_t _depth, Node* _parent) :
				m_quad(_quad),
				m_depth(_depth),
				m_tl(nullptr),
				m_tr(nullptr),
				m_bl(nullptr),
				m_br(nullptr),
				m_parent(_parent)
			{
			}

			~Node()
			{
				collapse();
			}

			void divide()
			{
				if (has_child()) return;

				m_tl = new Node(
					{ m_quad.center - m_quad.half_size / 2, m_quad.half_size / 2 },
					m_depth + 1,
					this);

				m_tr = new Node(
					{ m_quad.center + Point{ m_quad.half_size.x / 2, -(m_quad.half_size.y / 2) }, m_quad.half_size / 2 },
					m_depth + 1,
					this);

				m_bl = new Node(
					{ m_quad.center + Point{ -(m_quad.half_size.x / 2), m_quad.half_size.y / 2 }, m_quad.half_size / 2 },
					m_depth + 1,
					this);

				m_br = new Node(
					{ m_quad.center + m_quad.half_size / 2, m_quad.half_size / 2 },
					m_depth + 1,
					this);
			}

			void collapse()
			{
				release_child_node(m_tl);
				release_child_node(m_tr);
				release_child_node(m_bl);
				release_child_node(m_br);
			}

			Node* get_child_node_at(const Point& point)
			{
				if (point.x <= m_quad.center.x  &&
					point.x >= m_quad.center.x - m_quad.half_size.x)
				{
					if (point.y <= m_quad.center.y)
					{
						if (point.y >= m_quad.center.y - m_quad.half_size.y)
							return m_tl;
					}
					else
					{
						if (point.y <= m_quad.center.y + m_quad.half_size.y)
							return m_tr;
					}
				}
				else if (
					point.x >= m_quad.center.x  &&
					point.x <= m_quad.center.x + m_quad.half_size.x)
				{
					if (point.y <= m_quad.center.y)
					{
						if (point.y >= m_quad.center.y - m_quad.half_size.y)
							return m_bl;
					}
					else
					{
						if (point.y <= m_quad.center.y + m_quad.half_size.y)
							return m_br;
					}
				}

				return nullptr;
			}

			const Node* get_child_node_at(const Point& point) const
			{
				return const_cast<Node*>(this)->get_child_node_at(point);
			}

			const Quad& get_centred_quad() const { return m_quad; }

			const Node* get_node_at(const Point& point) const
			{
				const Node* node_at_point = get_child_node_at(point);
				
				if (node_at_point)
				{
					return node_at_point->get_node_at(point);
				}

				return this;
			}

			void query_nodes(std::vector<const Node*>& vec) const
			{
				if (!has_child())
				{
					vec.push_back(this);
					return;
				}

				m_tl->query_nodes(vec);
				m_tr->query_nodes(vec);
				m_bl->query_nodes(vec);
				m_br->query_nodes(vec);
			}
		};

	private:
		Node m_root;

		void divide(Point _where, size_t depth, Node& node)
		{
			if (depth == 0) return;

			Node* node_at_point = node.get_child_node_at(_where);
			if (!node_at_point)
			{
				node.divide();
				node_at_point = node.get_child_node_at(_where);
			}

			assert(node_at_point != nullptr);

			divide(_where, depth - 1, *node_at_point);
		}

	public:

		TerrainQuadTree(Quad quad) :
			m_root(quad, 0, nullptr)
		{
		};

		~TerrainQuadTree() {};

		bool divide(Point _where, size_t depth)
		{
			if (!m_root.get_centred_quad().contains(_where)) return false;
			divide(_where, depth, m_root);
			return true;
		}

		const Node* get_node_at(const Point& point) const
		{
			const Node* node_at_point = m_root.get_node_at(point);
			if (node_at_point)
			{
				return node_at_point;
			}

			return &m_root;
		}

		void query_nodes(std::vector<const Node*>& vec) const
		{
			vec.clear();
			m_root.query_nodes(vec);
		}

		void query_nodes(const Point& point, double radius) const
		{

		}
	};
}