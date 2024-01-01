/*
Jordan Dehmel
2023 - present
jdehmel@outlook.com
*/

#include "type_builder.hpp"
#include "sequence.hpp"
#include "sequence_resources.hpp"

std::set<std::string> deps;
std::map<std::string, __structLookupData> structData;
std::vector<std::string> structOrder;

unsigned long long currentID = 1;

typeNode &typeNode::operator=(const typeNode &other)
{
    info = other.info;
    name = other.name;
    return *this;
}

bool typeNode::operator==(const typeNode &other) const
{
    if (info != other.info)
    {
        return false;
    }
    else if (info == atomic)
    {
        return name == other.name;
    }
    else
    {
        return true;
    }
}

const Type nullType(atomic, "NULL");

Type::Type(const TypeInfo &Info, const std::string &Name)
{
    internal.clear();
    internal.push_back({Info, Name});
    ID = currentID++;
    return;
}

const size_t Type::size() const
{
    return internal.size();
}

Type::Type(const Type &What)
{
    internal.clear();
    for (const auto &i : What.internal)
    {
        internal.push_back(i);
    }
    ID = currentID++;

    return;
}

Type::Type(const Type &What, const int &startingAt)
{
    internal.clear();
    for (int i = startingAt; i < What.size(); i++)
    {
        internal.push_back(What.internal[i]);
    }
    ID = currentID++;

    return;
}

Type::Type()
{
    internal.clear();
    internal.push_back(typeNode{nullType.internal[0].info, nullType.internal[0].name});
    ID = currentID++;
    return;
}

void Type::prepend(const TypeInfo &Info, const std::string &Name)
{
    internal.insert(internal.begin(), {Info, Name});
    ID = currentID++;
    return;
}

void Type::append(const TypeInfo &Info, const std::string &Name)
{
    if (internal.size() == 1 && internal[0].info == atomic && internal[0].name == "NULL")
    {
        internal[0].info = Info;
        internal[0].name = Name;
    }
    else
    {
        internal.push_back({Info, Name});
    }
    ID = currentID++;

    return;
}

void Type::append(const Type &Other)
{
    if (internal.size() == 1 && internal[0].info == atomic && internal[0].name == "NULL")
    {
        internal[0] = Other.internal[0];
    }
    else
    {
        for (const auto &i : Other.internal)
        {
            internal.push_back(i);
        }
    }
    ID = currentID++;

    return;
}

void Type::reserve(const unsigned long long &To)
{
    internal.reserve(To);
}

bool Type::operator==(const Type &Other) const
{
    return internal == Other.internal;
}

bool Type::operator!=(const Type &Other) const
{
    return !(*this == Other);
}

Type &Type::operator=(const Type &Other)
{
    internal.clear();
    for (const auto &i : Other.internal)
    {
        internal.push_back(i);
    }
    ID = currentID++;
    return *this;
}

Type &Type::operator=(const typeNode &Other)
{
    internal.clear();
    internal.push_back(Other);
    ID = currentID++;
    return *this;
}

std::string toStr(const Type *const What, const unsigned int &pos)
{
    if (What == nullptr || pos >= What->internal.size())
    {
        return "";
    }

    std::string out = "";

    switch (What->internal[pos].info)
    {
    case arr:
        out += "[]";
        break;
    case sarr:
        out += "[" + What->internal[pos].name + "]";
        break;
    case pointer:
        out += "^";
        break;
    case join:
        out += ",";
        break;
    case maps:
        out += ") ->";
        break;
    case function:
        out += "(";
        break;
    case var_name:
        out += What->internal[pos].name + ":";
        break;
    default:
        out += What->internal[pos].name;
        break;
    }

    if (pos < What->internal.size())
    {
        if (What->internal[pos + 1].info != function && What->internal[pos + 1].info != pointer &&
            What->internal[pos + 1].info != join && What->internal[pos + 1].info != maps &&
            What->internal[pos].info != pointer && What->internal[pos].info != function)
        {
            out += " ";
        }
        else if (What->internal[pos].info == var_name)
        {
            out += " ";
        }

        out += toStr(What, pos + 1);
    }

    return out;
}

void Type::pop_front()
{
    internal.erase(internal.begin());
    ID = currentID++;
    return;
}

void Type::pop_back()
{
    internal.pop_back();
    ID = currentID++;
    return;
}

// Ignores all var_names
// As of 0.0.21, can also do automatic referencing
// of arguments
bool typesAreSame(const Type *const A, const Type *const B, int &changes)
{
    unsigned int left, right;
    left = right = 0;

    while (left < A->internal.size() && right < B->internal.size())
    {
        while (left < A->internal.size() && (A->internal[left].info == var_name || A->internal[left].info == pointer))
        {
            if (A->internal[left].info == pointer)
            {
                changes++;
            }

            left++;
        }

        while (right < B->internal.size() &&
               (B->internal[right].info == var_name || B->internal[right].info == pointer))
        {
            if (B->internal[right].info == pointer)
            {
                changes++;
            }

            right++;
        }

        if (left >= A->internal.size() || right >= B->internal.size())
        {
            break;
        }

        if (A->internal[left].info != B->internal[right].info &&
            !(A->internal[left].info == sarr && B->internal[right].info == arr) &&
            !(A->internal[left].info == arr && B->internal[right].info == sarr))
        {
            // Failure
            return false;
        }
        else
        {
            if (A->internal[left].info == atomic && A->internal[left].name != B->internal[right].name)
            {
                // Failure
                return false;
            }
        }

        // Success; Move on
        left++;
        right++;
    }

    return true;
}

// Like the above, but does not do auto-referencing or dereferencing
bool typesAreSameExact(const Type *const A, const Type *const B)
{
    unsigned int left, right;
    left = right = 0;

    while (left < A->internal.size() && right < B->internal.size())
    {
        while (left < A->internal.size() && A->internal[left].info == var_name)
        {
            left++;
        }

        while (right < B->internal.size() && B->internal[right].info == var_name)
        {
            right++;
        }

        if (left >= A->internal.size() || right >= B->internal.size())
        {
            break;
        }

        if (A->internal[left].info != B->internal[right].info &&
            !(A->internal[left].info == sarr && B->internal[right].info == arr) &&
            !(A->internal[left].info == arr && B->internal[right].info == sarr))
        {
            // Failure
            return false;
        }
        else
        {
            if (A->internal[left].info == atomic && A->internal[left].name != B->internal[right].name)
            {
                // Failure
                return false;
            }
        }

        // Success; Move on
        left++;
        right++;
    }

    return true;
}

Type checkLiteral(const std::string &From)
{
    // Check as bool
    if (From == "true" || From == "false")
    {
        return Type(atomic, "bool");
    }

    // Check as std::string
    else if ((From.front() == '"' && From.back() == '"') || (From.front() == '\'' && From.back() == '\''))
    {
        // Char (do not use)
        // if (From.size() == 3)
        // {
        //     return Type(atomic, "i8");
        // }

        // Str
        return Type(atomic, "str");
    }

    // Check as hex or binary literal
    if (From.size() > 2 && From.substr(0, 2) == "0b")
    {
        // Ensure validity
        for (int i = 2; i < From.size(); i++)
        {
            if (From[i] != '0' && From[i] != '1')
            {
                throw sequencing_error("Invalid binary literal '" + From + "'");
            }
        }

        // Single byte
        if (From.size() <= 10)
        {
            return Type(atomic, "u8");
        }

        // Two bytes
        else if (From.size() <= 18)
        {
            return Type(atomic, "u16");
        }

        // Four bytes
        else if (From.size() <= 34)
        {
            return Type(atomic, "u32");
        }

        // Eight bytes
        else if (From.size() <= 66)
        {
            return Type(atomic, "u64");
        }

        // Sixteen bytes
        else if (From.size() <= 130)
        {
            return Type(atomic, "u128");
        }

        // Too many bytes
        else
        {
            throw sequencing_error("Binary '" + From + "' cannot fit in integer literal.");
        }
    }
    else if (From.size() > 2 && From.substr(0, 2) == "0x")
    {
        // Ensure validity
        for (int i = 2; i < From.size(); i++)
        {
            if (!(('0' <= From[i] && From[i] <= '9') || ('a' <= From[i] && From[i] <= 'f') ||
                  ('A' <= From[i] && From[i] <= 'F')))
            {
                std::cout << "Due to char " << From[i] << '\n';
                throw sequencing_error("Invalid hex literal '" + From + "'");
            }
        }

        // Single byte
        if (From.size() <= 4)
        {
            return Type(atomic, "u8");
        }

        // Two bytes
        else if (From.size() <= 6)
        {
            return Type(atomic, "u16");
        }

        // Four bytes
        else if (From.size() <= 10)
        {
            return Type(atomic, "u32");
        }

        // Eight bytes
        else if (From.size() <= 18)
        {
            return Type(atomic, "u64");
        }

        // Sixteen bytes
        else if (From.size() <= 34)
        {
            return Type(atomic, "u128");
        }

        // Too many bytes
        else
        {
            throw sequencing_error("Hex '" + From + "' cannot fit in integer literal.");
        }
    }

    // Check as numbers
    bool canBeNumber = true;
    for (char c : From)
    {
        if (!('0' <= c && c <= '9') && c != '-' && c != '.')
        {
            canBeNumber = false;
            break;
        }
    }

    if (canBeNumber)
    {
        if (From.find(".") == std::string::npos)
        {
            // Int

            // Check size
            long long val = stoll(From);

            if (std::abs(val) <= __INT_MAX__)
            {
                return Type(atomic, "i32");
            }
            else if (std::abs(val) <= __LONG_MAX__)
            {
                return Type(atomic, "i64");
            }
            else
            {
                return Type(atomic, "i128");
            }
        }
        else
        {
            // Float
            return Type(atomic, "f64");
        }
    }

    // Default: Not a literal
    return nullType;
}

typeNode &Type::operator[](const int &Index) const
{
    return (typeNode &)internal[Index];
}
