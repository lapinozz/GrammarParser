#pragma once

#include "lexer.hpp"
#include "grammar.hpp"

class Parser
{
public:
	struct Error
	{
		Lexer::State state;
		std::wstring message;
	};

	Parser(Lexer& lexer, Grammar& grammar) : lexer(lexer), grammar(grammar) {}

	bool hasError() const
	{
		return !error.message.empty();
	}

	const Error& getError() const
	{
		return error;
	}

	bool parseEOF()
	{
		return lexer.consume(std::char_traits<wchar_t>::eof());
	}

	bool parseNonTerminal()
	{
		std::wstring symbol;
		if (lexer.consumeIdentifier(symbol))
		{
			auto& rule = grammar.back();
			if (rule.name.empty())
			{
				rule.name = symbol;
			}
			else
			{
				rule.symbols.emplace_back(Rule::NonTerminalSymbol{ symbol });
			}

			return true;
		}

		return false;
	}

	bool parseLiteralChar(wchar_t& c)
	{
		auto guard = lexer.guardState();

		c = lexer.consume();
		if (c == '\n')
		{
			return false;
		}
		else if (c == '\\')
		{
			c = lexer.consume();

			if (c == '\n')
			{
				return false;
			}

			if (c == 'n')
			{
				c = '\n';
			}
		}

		if (Lexer::isEOF(c))
		{
			return false;
		}

		guard.free();

		return true;
	}

	bool parseTerminal()
	{
		if (lexer.consume('['))
		{
			auto& rule = grammar.back();
			Rule::ChoiceTerminalSymbol range;

			std::wstring symbol;
			while (!lexer.consume(']', false))
			{
				wchar_t c;
				if (!parseLiteralChar(c))
				{
					setError(L"Expected `]` or a valid character literal");
					return false;
				}

				if (!lexer.peek(L"-]", false) && lexer.consume('-', false))
				{
					wchar_t c2;
					if (!parseLiteralChar(c2))
					{
						setError(L"Expected `]` or a valid character literal");
						return false;
					}

					range.push_back(Rule::RangeTerminalSymbol{ c, c2 });
				}
				else
				{
					range.push_back(Rule::LiteralTerminalSymbol{ c });
				}
			}

			rule.symbols.emplace_back(range);

			return true;
		}
		else if (lexer.consume('\''))
		{
			auto& rule = grammar.back();
			while (!lexer.consume('\'', false))
			{
				wchar_t c;
				if (!parseLiteralChar(c))
				{
					setError(L"Expected `]` or a valid character literal");
					return false;
				}

				rule.symbols.push_back(Rule::LiteralTerminalSymbol{ c });
			}

			return true;
		}

		return false;
	}

	bool parseSymbol()
	{
		return parseNonTerminal() || parseTerminal();
	}

	bool parseRuleAssign()
	{
		return lexer.consume(L"->");
	}

	bool parseRule()
	{
		grammar.push_back({});

		if (!parseNonTerminal())
		{
			setError(L"Expected a non-terminal symbol");
			return false;
		}

		if (!parseRuleAssign())
		{
			setError(L"Expected нннннн`->`");
			return false;
		}

		while (true)
		{
			auto guard = lexer.guardState();

			if (parseEOF())
			{
				break;
			}

			if (lexer.consume('|'))
			{
				grammar.push_back({ grammar.back().name });
				guard.free();
				continue;
			}

			if (!parseSymbol())
			{
				if (!hasError())
				{
					setError(L"Expected Non-Terminal Symbol or Terminal Symbol");
				}
				return false;
			}

			if (parseRuleAssign())
			{
				grammar.back().symbols.pop_back();
				guard.pop();
				break;
			}

			guard.free();
		}

		return true;
	}

	bool parseGrammar()
	{
		while (true)
		{
			if (parseEOF())
			{
				return true;
			}

			if (!parseRule())
			{
				return false;
			}
		}
	}

private:
	void setError(const std::wstring message)
	{
		error = { lexer.getState(), message };
	}

	Lexer& lexer;
	Grammar& grammar;
	Error error;
};