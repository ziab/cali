#pragma once
#include <map>

namespace cali
{
	template <typename... Args>
	class signal
	{
	public:

		signal() : current_id_(0) {}

		// copy creates new signal
		signal(signal const& other) : current_id_(0) {}

		// connects a member function to this signal
		template <typename T>
		int connect_member(T *inst, void (T::*func)(Args...)) {
			return connect([=](Args... args) {
				(inst->*func)(args...);
			});
		}

		// connects a const member function to this signal
		template <typename T>
		int connect_member(T *inst, void (T::*func)(Args...) const) {
			return connect([=](Args... args) {
				(inst->*func)(args...);
			});
		}

		// connects a std::function to the signal. The returned
		// value can be used to disconnect the function again
		int connect(std::function<void(Args...)> const& slot) const {
			slots_.insert(std::make_pair(++current_id_, slot));
			return current_id_;
		}

		// disconnects a previously connected function
		void disconnect(int id) const {
			slots_.erase(id);
		}

		// disconnects all previously connected functions
		void disconnect_all() const {
			slots_.clear();
		}

		// calls all connected functions
		void emit(Args... p) {
			for (auto it : slots_) {
				it.second(p...);
			}
		}

		// assignment creates new signal
		signal& operator=(signal const& other) {
			disconnect_all();
		}

	private:
		mutable std::map<int, std::function<void(Args...)>> slots_;
		mutable int current_id_;
	};
}