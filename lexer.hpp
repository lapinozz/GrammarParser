#pragma once

#include <string>
#include <vector>
#include <sstream>

class Lexer
{
	class StateGuard
	{
	public:
		StateGuard(Lexer& lexer) : lexer(lexer)
		{
			lexer.pushState();
		}

		void pop()
		{
			if (isFreed)
			{
				return;
			}

			lexer.popState();
			isFreed = true;
		}

		void free()
		{
			if (isFreed)
			{
				return;
			}

			lexer.freeState();
			isFreed = true;
		}

		~StateGuard()
		{
			pop();
		}

	private:
		Lexer& lexer;
		bool isFreed = false;
	};

public:
	struct State
	{
		std::streamoff offset;
		size_t line;
		size_t column;
	};

	Lexer(std::wistream& stream) : stream(stream) {}

	static bool isWhitespace(wchar_t c)
	{
		return (c <= 32) | (c == 127);
	}

	static bool isAlpha(wchar_t c)
	{
		return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
	}

	static bool isDigit(wchar_t c)
	{
		return (c >= '0') && (c <= '9');
	}

	static bool isIdentifier(wchar_t c, bool firstChar = false)
	{
		return c == '_' || c == '-' || isAlpha(c) || (!firstChar && isDigit(c));
	}

	static bool isEOF(wchar_t c)
	{
		return c == std::char_traits<wchar_t>::eof();
	}

	const State& getState() const
	{
		return state;
	}

	void pushState()
	{
		states.push_back(state);
	}

	void popState()
	{
		state = states.back();
		stream.seekg(state.offset, std::ios_base::beg);
		states.pop_back();
	}

	void freeState()
	{
		states.pop_back();
	}

	StateGuard guardState()
	{
		return { *this };
	}

	wchar_t peek() const
	{
		return stream.peek();
	}

	wchar_t consume()
	{
		const auto c = stream.get();
		state.offset = stream.tellg();

		if (c == '\n')
		{
			state.line++;
			state.column = 0;
		}
		else
		{
			state.column++;
		}

		return c;
	}

	bool peek(const std::wstring& str, bool bConsumeWhitespaces = true)
	{
		auto guard = guardState();

		if (bConsumeWhitespaces)
		{
			consumeWhitespaces();
		}

		for (const auto c : str)
		{
			if (consume() != c)
			{
				return false;
			}
		}

		return true;
	}

	bool consume(const std::wstring& str, bool bConsumeWhitespaces = true)
	{
		auto guard = guardState();

		if (bConsumeWhitespaces)
		{
			consumeWhitespaces();
		}

		for (const auto c : str)
		{
			if (consume() != c)
			{
				return false;
			}
		}

		guard.free();
		return true;
	}

	bool peek(wchar_t c, bool bConsumeWhitespaces = true)
	{
		auto guard = guardState();

		if (bConsumeWhitespaces)
		{
			consumeWhitespaces();
		}

		return peek() == c;
	}

	bool consume(wchar_t c, bool bConsumeWhitespaces = true)
	{
		auto guard = guardState();

		if (bConsumeWhitespaces)
		{
			consumeWhitespaces();
		}

		if (consume() == c)
		{
			guard.free();
			return true;
		}

		return false;
	}

	bool consumeIdentifier(std::wstring& out, bool bConsumeWhitespaces = true)
	{
		auto guard = guardState();

		if (bConsumeWhitespaces)
		{
			consumeWhitespaces();
		}

		if (isIdentifier(peek(), true))
		{
			out = consume();
		}

		while (isIdentifier(peek()))
		{
			out += consume();
		}

		if (out.size() > 0)
		{
			guard.free();
			return true;
		}

		return false;
	}

	void consumeWhitespaces()
	{
		while (isWhitespace(peek()))
		{
			consume();
		}
	}

private:
	std::vector<State> states;

	State state{};
	std::wistream& stream;
};