/*
Jordan Dehmel
2023 - present
jdehmel@outlook.com
*/

#include "oakc_fns.hpp"
#include "oakc_structs.hpp"
#include "options.hpp"
#include <string>

static unsigned long long currentID = 1;

TypeNode &TypeNode::operator=(const TypeNode &other)
{
    info = other.info;
    name = other.name;
    return *this;
}

bool TypeNode::operator==(const TypeNode &other) const
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
    int i = 0;
    for (const auto &item : What.internal)
    {
        if (i >= startingAt)
        {
            internal.push_back(item);
        }
        i++;
    }
    ID = currentID++;

    return;
}

Type::Type()
{
    internal.clear();
    internal.push_back(TypeNode{nullType.internal.front().info, nullType.internal.front().name});
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
    if (internal.size() == 1 && internal.front().info == atomic && internal.front().name == "NULL")
    {
        internal.front().info = Info;
        internal.front().name = Name;
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
    if (internal.size() == 1 && internal.front().info == atomic && internal.front().name == "NULL")
    {
        internal.front() = Other.internal.front();
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

Type &Type::operator=(const TypeNode &Other)
{
    internal.clear();
    internal.push_back(Other);
    ID = currentID++;
    return *this;
}

void toStr(const Type *const What, const std::list<TypeNode>::const_iterator &pos, std::list<std::string> &builder)
{
    if (What == nullptr || pos == What->internal.end())
    {
        return;
    }

    switch (pos->info)
    {
    case arr:
        builder.push_back("[]");
        break;
    case sarr:
        builder.push_back("[");
        builder.push_back(pos->name);
        builder.push_back("]");
        break;
    case pointer:
        builder.push_back("^");
        break;
    case join:
        builder.push_back(",");
        break;
    case maps:
        builder.push_back(") ->");
        break;
    case function:
        builder.push_back("(");
        break;
    case var_name:
        builder.push_back(pos->name);
        builder.push_back(":");
        break;
    default:
        builder.push_back(pos->name);
        break;
    }

    if (std::next(pos) != What->internal.end())
    {
        if (std::next(pos)->info != function && std::next(pos)->info != pointer && std::next(pos)->info != join &&
            std::next(pos)->info != maps && pos->info != pointer && pos->info != function)
        {
            builder.push_back(" ");
        }
        else if (pos->info == var_name)
        {
            builder.push_back(" ");
        }

        toStr(What, std::next(pos), builder);
    }

    return;
}

// Return the standard C representation of this type.
std::string toStr(const Type *const what)
{
    std::list<std::string> builder;

    toStr(what, what->internal.begin(), builder);

    std::string out;
    size_t size = 0;
    for (const auto &item : builder)
    {
        size += item.size();
    }

    out.reserve(size);

    for (const auto &item : builder)
    {
        out += item;
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
    auto left = A->internal.begin();
    auto right = B->internal.begin();

    while (left != A->internal.end() && right != B->internal.end())
    {
        while (left != A->internal.end() && (left->info == var_name || left->info == pointer))
        {
            if (left->info == pointer)
            {
                changes++;
            }

            left++;
        }

        while (right != B->internal.end() && (right->info == var_name || right->info == pointer))
        {
            if (right->info == pointer)
            {
                changes++;
            }

            right++;
        }

        if (left == A->internal.end() || right == B->internal.end())
        {
            break;
        }

        if (left->info != right->info && !(left->info == sarr && right->info == arr) &&
            !(left->info == arr && right->info == sarr))
        {
            // Failure
            return false;
        }
        else
        {
            if (left->info == atomic && left->name != right->name)
            {
                // Failure
                return false;
            }
        }

        // Success; Move on
        left++;
        right++;
    }

    if (left == A->internal.end() || right == B->internal.end())
    {
        if (!(left == A->internal.end() && right == B->internal.end()))
        {
            return false;
        }
    }

    return true;
}

// Like the above, but does not do auto-referencing or dereferencing
bool typesAreSameExact(const Type *const A, const Type *const B)
{
    auto left = A->internal.begin();
    auto right = B->internal.begin();

    while (left != A->internal.end() && right != B->internal.end())
    {
        while (left != A->internal.end() && left->info == var_name)
        {
            left++;
        }

        while (right != B->internal.end() && right->info == var_name)
        {
            right++;
        }

        if (left == A->internal.end() || right == B->internal.end())
        {
            return false;
        }

        if (left->info != right->info && !(left->info == sarr && right->info == arr) &&
            !(left->info == arr && right->info == sarr))
        {
            // Failure
            return false;
        }
        else
        {
            if (left->info == atomic && left->name != right->name)
            {
                // Failure
                return false;
            }
        }

        // Success; Move on
        left++;
        right++;
    }

    if (left == A->internal.end() || right == B->internal.end())
    {
        if (!(left == A->internal.end() && right == B->internal.end()))
        {
            return false;
        }
    }

    return true;
}

/*
Compares two types. Returns true if they match exactly, if they
match using auto-reference/auto-dereference, or internal literal
casting. The number of changes is recorded in `changes`.

A is passed, B is candidate
*/
bool typesAreSameCast(const Type *const passed, const Type *const candidate, int &changes)
{
    bool castIsLegal = true;

    auto left = passed->internal.begin();
    auto right = candidate->internal.begin();

    while (left != passed->internal.end() && right != candidate->internal.end())
    {
        while (left != passed->internal.end() && (left->info == var_name || left->info == pointer))
        {
            if (left->info == pointer)
            {
                castIsLegal = false;
                changes++;
            }
            else
            {
                castIsLegal = true;
            }

            left++;
        }

        while (right != candidate->internal.end() && (right->info == var_name || right->info == pointer))
        {
            if (right->info == pointer)
            {
                castIsLegal = false;
                changes++;
            }
            else
            {
                castIsLegal = true;
            }

            right++;
        }

        if (left == passed->internal.end() || right == candidate->internal.end())
        {
            break;
        }

        if (left->info != right->info && !(left->info == sarr && right->info == arr) &&
            !(left->info == arr && right->info == sarr))
        {
            // Failure
            return false;
        }
        else
        {
            if (left->info == atomic)
            {
                if (castIsLegal)
                {
                    castIsLegal = false;

                    // Case 1: Int cast
                    if (INT_LITERALS.count(left->name) != 0 && INT_LITERALS.count(right->name) != 0 &&
                        INT_LITERALS.at(left->name) <= INT_LITERALS.at(right->name))
                    {
                        changes++;
                        castIsLegal = true;
                    }

                    // Case 2: Float cast
                    if (!castIsLegal &&
                        (FLOAT_LITERALS.count(left->name) != 0 && FLOAT_LITERALS.count(right->name) != 0) &&
                        FLOAT_LITERALS.at(left->name) <= FLOAT_LITERALS.at(right->name))
                    {
                        changes++;
                        castIsLegal = true;
                    }
                }

                // Failure: No cast available
                if (!castIsLegal && left->name != right->name)
                {
                    return false;
                }
            }
        }

        // Success; Move on
        left++;
        right++;
    }

    if (left == passed->internal.end() || right == candidate->internal.end())
    {
        if (!(left == passed->internal.end() && right == candidate->internal.end()))
        {
            return false;
        }
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
    else if (From.size() >= 2 && From.substr(0, 2) == "0x")
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

    // Check as numbers w/o prefixes
    bool canBeNumber = (('0' <= From[0] && From[0] <= '9') || From[0] == '-');

    if (canBeNumber)
    {
        int i = 0;
        for (char c : From)
        {
            if (c == 'u')
            {
                // Unsigned literal suffix or invalid notation
                const char *suffix = &From.c_str()[i];
                if (strcmp(suffix, "u8") == 0)
                {
                    return Type(atomic, suffix);
                }
                else if (strcmp(suffix, "u16") == 0)
                {
                    return Type(atomic, suffix);
                }
                else if (strcmp(suffix, "u32") == 0)
                {
                    return Type(atomic, suffix);
                }
                else if (strcmp(suffix, "u64") == 0)
                {
                    return Type(atomic, suffix);
                }
                else if (strcmp(suffix, "u128") == 0)
                {
                    return Type(atomic, suffix);
                }

                throw sequencing_error("Invalid type-specifying suffix '" + std::string(suffix) + "'");
            }
            else if (c == 'i')
            {
                // Signed literal suffix or invalid notation
                const char *suffix = &From.c_str()[i];
                if (strcmp(suffix, "i8") == 0)
                {
                    return Type(atomic, suffix);
                }
                else if (strcmp(suffix, "i16") == 0)
                {
                    return Type(atomic, suffix);
                }
                else if (strcmp(suffix, "i32") == 0)
                {
                    return Type(atomic, suffix);
                }
                else if (strcmp(suffix, "i64") == 0)
                {
                    return Type(atomic, suffix);
                }
                else if (strcmp(suffix, "i128") == 0)
                {
                    return Type(atomic, suffix);
                }

                throw sequencing_error("Invalid type-specifying suffix '" + std::string(suffix) + "'");
            }
            else if (c == 'f')
            {
                // Float literal suffix or invalid notation
                // Unsigned literal suffix or invalid notation
                const char *suffix = &From.c_str()[i];
                if (strcmp(suffix, "f32") == 0)
                {
                    return Type(atomic, suffix);
                }
                else if (strcmp(suffix, "f64") == 0)
                {
                    return Type(atomic, suffix);
                }

                throw sequencing_error("Invalid type-specifying suffix '" + std::string(suffix) + "'");
            }

            // Must be floating point to use '.'
            else if (c == '.')
            {
                // Float
                return Type(atomic, "f64");
            }

            if (!('0' <= c && c <= '9') && c != '-')
            {
                return nullType;
            }

            i++;
        }

        long long val;
        unsigned long long uval;
        bool viable = true;

        try
        {
            val = stoll(From);
        }
        catch (...)
        {
            viable = false;
        }

        if (viable)
        {
            if (INT_MIN <= val && val <= INT_MAX)
            {
                return Type(atomic, "i32");
            }
            else if (LONG_MIN <= val && val <= LONG_MIN)
            {
                return Type(atomic, "i64");
            }
            else if (LONG_LONG_MIN <= val && val <= LONG_LONG_MAX)
            {
                return Type(atomic, "i128");
            }
        }

        viable = true;
        try
        {
            uval = stoull(From);
        }
        catch (...)
        {
            viable = false;
        }

        if (viable)
        {
            if (uval <= UINT_MAX)
            {
                return Type(atomic, "u32");
            }
            else if (uval <= ULONG_MAX)
            {
                return Type(atomic, "u64");
            }
            else if (uval <= ULONG_LONG_MAX)
            {
                return Type(atomic, "u128");
            }
        }

        throw sequencing_error("Invalid numerical literal '" + From + "'- Out of range of i128 and u128.");
    }

    // Default: Not a literal
    return nullType;
}

// Costly! Beware!
TypeNode &Type::operator[](const int &Index) const
{
    return (TypeNode &)*std::next(internal.begin(), Index);
}
