#include <string>
#include <vector>
#include <iostream>
#include <regex>

// ====================================================

inline bool replace(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

inline void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = str.find(from);
    while (start_pos != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos = str.find(from, start_pos + to.length());
    }
}

inline std::string RegExEscape(std::string str)
{
#if 0
#define x(x) replaceAll(str, x, "\\" x)
    x("\\");
    x("^");
    x("$");
    x(".");
    x("|");
    x("?");
    x("*");
    x("+");
    x("(");
    x(")");
    x("[");
    x("]");
    x("{");
    x("}");
#undef x
#elif 0
    for (char c : "\\^$.|?*+()[]{}")
    {
        size_t start_pos = str.find(c);
        while (start_pos != std::string::npos)
        {
            str.insert(start_pos, 1, '\\');
            start_pos = str.find(c, start_pos + 2);
        }
    }
#elif 0
    // count all special characters
    // increase str length by count
    // iterate backwards though str
    //   moving each character to its final place
    //   and inserting the escape character as appropriate
#else
    const char special[] = "\\^$.|?*+()[]{}";
    size_t start_pos = str.find_first_of(special);
    while (start_pos != std::string::npos)
    {
        str.insert(start_pos, 1, '\\');
        start_pos = str.find_first_of(special, start_pos + 2);
    }
#endif
    return str;
}

// ====================================================

class TokenType
{
public:
    TokenType(const std::string& name, const std::string& s, bool ignore = false)
        : m_name(name), m_re("^" + s), m_ignore(ignore)
    {
    }

    std::string m_name;
    std::regex m_re;
    bool m_ignore;
};

struct Token
{
    const TokenType* type;
    std::string str;
};

class Tokenizer
{
public:
    Tokenizer(const std::vector<TokenType>& res, const std::string& s)
        : m_res(res), m_itstart(s.begin()), m_itend(s.end())
    {}

    bool more()
    {
        return m_itstart != m_itend;
    }

    Token getNextToken()
    {
        Token token = {};

        do
        {
            token = {};
            const std::string::const_iterator itstart = m_itstart;
            for (const auto& tokentype : m_res)
            {
                std::smatch match;
                if (std::regex_search(itstart, m_itend, match, tokentype.m_re))
                {
                    //_ASSERTE(match.prefix().length() == 0);
                    if (match.prefix().length() == 0 && static_cast<size_t>(match.length()) > token.str.length())
                    {
                        token.type = &tokentype;
                        token.str = match[0];
                        m_itstart = match[0].second;
                    }
                }
            }
        } while (token.type != nullptr and token.type->m_ignore);

        return token;
    }

    std::string getRest() const { return std::string(m_itstart, m_itend); }

private:
    const std::vector<TokenType>& m_res;
    std::string::const_iterator m_itstart;
    const std::string::const_iterator m_itend;
};

int main(const int argc, const char* const argv[])
{
    const std::vector<TokenType> res = {
        TokenType("Keyword", "break|const|if|nullptr|while"),
        TokenType("Word", "[a-zA-Z]\\w*"),
        TokenType("Number", "\\d+"),
        TokenType("String", R"-("[^"]*")-"),
        TokenType("Shift", "<<"),
        TokenType("Deref", "->"),
        TokenType("Scope", "::"),
        TokenType("Punctuation", "[" + RegExEscape("[](){}.=:;") + "]"),
        TokenType("Space", "\\s+", true),
    };

    const std::string s(
        R"-(while (tz.more()))-" "\n"
        R"-({)-" "\n"
        R"-(    const Token t = tz.getNextToken();)-" "\n"
        R"-(    if (t.type == nullptr))-" "\n"
        R"-(        break;)-" "\n"
        R"-(    std::cout << t.type->name() << " " << t.str << "\n";)-" "\n"
        R"-(})-"  "\n"
    );

    std::cout << s << "\n";
    std::cout << "---\n";

    Tokenizer tz(res, s);

    while (tz.more())
    {
        const Token t = tz.getNextToken();
        if (t.type == nullptr)
            break;
        std::cout << t.type->m_name << "\t" << t.str << "\n";
    }

    std::cout << "---\n";
    std::cout << tz.getRest() << "\n";

    return EXIT_SUCCESS;
}
