#pragma once
#include <cstdio>

namespace command {
	namespace detail
	{
		template<typename T>
		T ConvertArgument(std::string_view view)
		{
			if constexpr(std::is_same_v<T, std::string_view>)
				return view;
			else if constexpr(std::is_same_v<T, uint32_t> || std::is_same_v<T, int>)
				return std::stol(std::string(view));
			else if constexpr(std::is_same_v<T, bool>)
				return view != "0";
		}

		template<typename T>
		struct arg_count;

		template<typename Ret, typename... Args>
		struct arg_count<Ret(Args...)>
		{
			constexpr static auto size = sizeof...(Args);
		};

		template<std::size_t N, typename T>
		struct nth_type;

		template<std::size_t N, typename Ret, typename... Args>
		struct nth_type<N, Ret(Args...)>
		{
			using type = typename std::tuple_element<N, std::tuple<Args...> >::type;
		};
		template<std::size_t N, typename T>
		using nth_type_t = typename nth_type<N, T>::type;
		
		template<bool defaultOptional = false, typename T, typename... Ts>
		void CallCommand(const std::vector<std::string_view>& args, ksignals::Event<T>* func, Ts... ts)
		{
			if constexpr(sizeof...(Ts) == arg_count<T>::size)
				(*func)(ts...);
			else
			{
				std::string_view arg = "";
				if constexpr(defaultOptional) {
				
					if (args.size() > 1 + sizeof...(Ts))
						arg = args[1 + sizeof...(Ts)];
				}
				else
					arg = args[1 + sizeof...(Ts)];
				CallCommand<defaultOptional>(args, func, std::forward<Ts>(ts)..., ConvertArgument<nth_type_t<sizeof...(Ts), T>>(arg));
			}
				
		}

	}

	template<typename T, typename... Ts>
	void RunCommand(const std::vector<std::string_view>& args, std::string name, ksignals::Event<T>* func, Ts... ts)
	{
		if (name == args[0]) {
			return detail::CallCommand(args, func);
		}
		if constexpr(sizeof...(Ts) != 0)
			return RunCommand(args, std::forward<Ts>(ts)...);
	}

	template<typename T, typename... Ts>
	void RunCommandOptional(const std::vector<std::string_view>& args, std::string name, ksignals::Event<T>* func, Ts... ts)
	{
		if (name == args[0])
			return detail::CallCommand<true>(args, func);
		if constexpr(sizeof...(Ts) != 0)
			return RunCommandOptional(args, ts...);
	}
}