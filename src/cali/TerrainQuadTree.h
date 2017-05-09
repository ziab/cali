#pragma once
#include <vector>
#include <exception>
#include <assert.h>
#include <functional>

namespace Cali
{
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

		double width() const { return half_size.x * 2.0; }
		double height() const { return half_size.y * 2.0; }
	};

	struct Circle
	{
		Point center;
		double radius;
		Circle(Point _center, double _radius) : center(_center), radius(_radius) {};

		// NOTE: quad is supposed to be axis-aligned
		inline bool intersects(const Quad& quad) const
		{
			double x_dist = abs(center.x - quad.center.x);
			double y_dist = abs(center.y - quad.center.y);

			if (x_dist > (quad.half_size.x + radius)) return false;
			if (y_dist > (quad.half_size.y + radius)) return false;

			if (x_dist <= (quad.half_size.x)) return true;
			if (y_dist <= (quad.half_size.y)) return true;

			double corner_distance_sq = (x_dist - quad.half_size.x * 2.0) * (x_dist - quad.half_size.x * 2.0) +
				(y_dist - quad.half_size.y * 2.0) * (quad.half_size.y * 2.0);

			return (corner_distance_sq <= (radius * radius));
		}

		Circle operator* (double x) const { return Circle{ center, radius * x }; }
	};

	class TerrainQuadTree
	{
	public:
		struct Node;
		typedef void VisitorCallback(const Node& node, void* private_data);

		struct Node
		{
		private:
			Quad m_quad;
			int m_depth;
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

			bool is_leaf() const
			{
				if (!m_tl && !m_tr && !m_bl && !m_br) return true;
				assert(m_tl || m_tr || m_bl || m_br);
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

			Node(Quad _quad, int _depth, Node* _parent) :
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
				if (!is_leaf()) return;

				m_tl = new Node(
					{ m_quad.center + Point{ -(m_quad.half_size.x / 2), m_quad.half_size.y / 2 }, m_quad.half_size / 2 },
					m_depth + 1,
					this);

				m_bl = new Node(
					{ m_quad.center - m_quad.half_size / 2, m_quad.half_size / 2 },
					m_depth + 1,
					this);

				m_tr = new Node(
					{ m_quad.center + m_quad.half_size / 2, m_quad.half_size / 2 },
					m_depth + 1,
					this);

				m_br = new Node(
					{ m_quad.center + Point{ m_quad.half_size.x / 2, -(m_quad.half_size.y / 2) }, m_quad.half_size / 2 },
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
				// left
				if (point.x <= m_quad.center.x  &&
					point.x >= m_quad.center.x - m_quad.half_size.x)
				{
					if (point.y >= m_quad.center.y)
					{
						if (point.y <= m_quad.center.y + m_quad.half_size.y)
							return m_tl;
					}
					else
					{
						if (point.y >= m_quad.center.y - m_quad.half_size.y)
							return m_bl;
					}
				}
				//right
				else if (
					point.x >= m_quad.center.x  &&
					point.x <= m_quad.center.x + m_quad.half_size.x)
				{
					if (point.y >= m_quad.center.y)
					{
						if (point.y <= m_quad.center.y + m_quad.half_size.y)
							return m_tr;
					}
					else
					{
						if (point.y >= m_quad.center.y - m_quad.half_size.y)
							return m_br;
					}
				}

				return nullptr;
			}

			typedef Node* QuadNodes[4];

			void get_child_nodes_at(const Circle& circle, QuadNodes& nodes) const
			{
				memset(&nodes, 0, sizeof(QuadNodes));

				if (circle.intersects(m_tl->m_quad)) nodes[0] = m_tl;
				if (circle.intersects(m_tr->m_quad)) nodes[1] = m_tr;
				if (circle.intersects(m_bl->m_quad)) nodes[2] = m_bl;
				if (circle.intersects(m_br->m_quad)) nodes[3] = m_br;
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

			void get_nodes(std::vector<const Node*>& vec) const
			{
				if (is_leaf())
				{
					vec.push_back(this);
					return;
				}

				m_tl->get_nodes(vec);
				m_tr->get_nodes(vec);
				m_bl->get_nodes(vec);
				m_br->get_nodes(vec);
			}

			void get_nodes_inside(const Circle& circle, std::vector<const Node*>& vec) const
			{
				if (is_leaf())
				{
					vec.push_back(this);
					return;
				}

				QuadNodes selected_child_nodes;
				get_child_nodes_at(circle, selected_child_nodes);

				for (auto* node : selected_child_nodes)
				{
					if (node) node->get_nodes_inside(circle, vec);
				}
			}

			void visit(const Circle& circle, const std::function<VisitorCallback>& callback, void* private_data) const
			{
				// this is the leaf node therefore we call visitor
				if (is_leaf())
				{
					callback(*this, private_data);
					return;
				}

				QuadNodes selected_child_nodes;
				get_child_nodes_at(circle, selected_child_nodes);

				for (auto* node : selected_child_nodes)
				{
					if (node) node->visit(circle, callback, private_data);
				}
			}

			template<typename TObject>
			void visit(
				const Circle& circle,
				TObject& object,
				void (TObject::*callback)(const Node&, void* render_context),
				void* private_data = nullptr) const
			{
				// this is the leaf node therefore we call visitor
				if (is_leaf())
				{
					(object.*callback)(*this, private_data);
					return;
				}

				QuadNodes selected_child_nodes;
				get_child_nodes_at(circle, selected_child_nodes);

				for (auto* node : selected_child_nodes)
				{
					if (node) node->visit(circle, object, callback, private_data);
				}
			}

			double get_scale_factor()
			{
				return 1.0 / m_depth;
			}

			void divide(Point _where, int depth)
			{
				if (depth <= 0) return;

				if (is_leaf()) divide();

				Node* node_at_point = get_child_node_at(_where);
				assert(node_at_point != nullptr);

				node_at_point->divide(_where, depth - 1);
			}

			void divide(const Circle& circle, int depth)
			{
				if (depth <= 0) 
					return;

				if (is_leaf()) divide();

				QuadNodes nodes;
				get_child_nodes_at(circle, nodes);

				for (auto* node : nodes)
				{
					if (!node) continue;
					node->divide(circle, depth - 1);
				}
			}
		};

	private:
		Node m_root;

	public:

		TerrainQuadTree(Quad quad) :
			m_root(quad, 1, nullptr)
		{
		};

		~TerrainQuadTree() {};

		bool divide(Point _where, int depth)
		{
			if (!m_root.get_centred_quad().contains(_where)) return false;
			m_root.divide(_where, depth);
			return true;
		}

		bool divide(Circle _where, int depth)
		{
			if (!_where.intersects(m_root.get_centred_quad())) return false;
			m_root.divide(_where, depth);
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

		void get_nodes(std::vector<const Node*>& vec) const
		{
			vec.clear();
			m_root.get_nodes(vec);
		}

		void get_nodes_inside(const Circle& circle, std::vector<const Node*>& vec) const
		{
			vec.clear();
			m_root.get_nodes_inside(circle, vec);
		}

		void collapse()
		{
			m_root.collapse();
		}

		void visit(const Circle& circle, const std::function<VisitorCallback>& callback, void* private_data = nullptr)
		{
			m_root.visit(circle, callback, private_data);
		}

		/// using std::function has some overhead so this method is supposed to work faster
		template<typename TObject>
		void visit(
			const Circle& circle, 
			TObject& object, 
			void (TObject::*callback)(const Node&, void* render_context),
			void* private_data = nullptr) const
		{
			m_root.visit<TObject>(circle, object, callback, private_data);
		}

		double width() const { return m_root.get_centred_quad().width(); };
		double height() const { return m_root.get_centred_quad().height(); };
	};
}