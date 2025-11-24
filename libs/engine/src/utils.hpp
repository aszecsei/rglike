//
// Created by Alic Szecsei on 11/24/2025.
//

#pragma once

// Source - https://stackoverflow.com/a
// Posted by Passer By, modified by community. See post 'Timeline' for change history
// Retrieved 2025-11-24, License - CC BY-SA 4.0

#include<type_traits>
#include<utility>
#include<new>

namespace engine::utils {
    template<int, typename Callable, typename Ret, typename... Args>
    auto fnptr_(Callable&& c, Ret (*)(Args...))
    {
        static std::decay_t<Callable> storage = std::forward<Callable>(c);
        static bool used = false;
        if(used)
        {
            using type = decltype(storage);
            storage.~type();
            new (&storage) type(std::forward<Callable>(c));
        }
        used = true;

        return [](Args... args) -> Ret {
            auto& c = *std::launder(&storage);
            return Ret(c(std::forward<Args>(args)...));
        };
    }

    template<typename Fn, int N = 0, typename Callable>
    Fn* fnptr(Callable&& c)
    {
        return fnptr_<N>(std::forward<Callable>(c), (Fn*)nullptr);
    }
}