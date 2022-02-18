
#ifndef _DIGILOG_ERR_HANDLING_HPP_
#define _DIGILOG_ERR_HANDLING_HPP_



#define critical(code, caption, fmt, ...) __critical(code, __FILE__, __LINE__, caption, fmt, __VA_ARGS__)
[[noreturn]] void __critical(int code, const char* filename, int line, const char* caption = nullptr, const char* fmt = "", ...);

#define critical_assert(true_expr, code, caption)		if (!(true_expr)) critical(code, caption, "Fatal error: critical assertion failed:\n" #true_expr); else ksn::nop()
#define critical_assert1(true_expr, code, caption, fmt, ...)			if (!(true_expr)) critical(code, caption, fmt, __VA_ARGS__); else ksn::nop()



#endif //!_DIGILOG_ERR_HANDLING_HPP_
