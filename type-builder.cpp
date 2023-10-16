#include "type-builder.hpp"

set<string> deps;
map<string, __structLookupData> structData;
vector<string> structOrder;

unsigned long long currentID = 1;

typeNode &typeNode::operator=(const typeNode &other)
{
    info = other.info;
    name = other.name;
    return *this;
}

const Type nullType(atomic, "NULL");

Type::Type(const TypeInfo &Info, const string &Name)
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

void Type::prepend(const TypeInfo &Info, const string &Name)
{
    internal.insert(internal.begin(), {Info, Name});
    ID = currentID++;
    return;
}

void Type::append(const TypeInfo &Info, const string &Name)
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

bool Type::operator==(const Type &Other) const
{
    for (int cur = 0; cur < internal.size(); cur++)
    {
        if (cur >= Other.size())
        {
            return false;
        }

        if (internal[cur].info != Other.internal[cur].info)
        {
            return false;
        }
        else if (internal[cur].info == atomic && internal[cur].name != Other.internal[cur].name)
        {
            return false;
        }
    }

    return true;
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

string toStr(const Type *const What, const unsigned int &pos)
{
    if (What == nullptr || pos >= What->internal.size())
    {
        return "";
    }

    string out = "";

    switch (What->internal[pos].info)
    {
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
bool typesAreSame(Type *A, Type *B)
{
    unsigned int left, right;
    left = right = 0;

    while (left < A->internal.size() && right < B->internal.size())
    {
        while (left < A->internal.size() && (A->internal[left].info == var_name || A->internal[left].info == pointer))
        {
            left++;
        }

        while (right < B->internal.size() && (B->internal[right].info == var_name || B->internal[right].info == pointer))
        {
            right++;
        }

        if (left >= A->internal.size() || right >= B->internal.size())
        {
            break;
        }

        if (A->internal[left].info != B->internal[right].info)
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

Type checkLiteral(const string &From)
{
    // Check as bool
    if (From == "true" || From == "false")
    {
        return Type(atomic, "bool");
    }

    // Check as string
    else if ((From.front() == '"' && From.back() == '"') || (From.front() == '\'' && From.back() == '\''))
    {
        return Type(atomic, "str");
    }

    // Check as numbers
    bool canBeDouble = true;
    for (char c : From)
    {
        if (!('0' <= c && c <= '9') && c != '-' && c != '.')
        {
            canBeDouble = false;
            break;
        }
    }

    if (canBeDouble)
    {
        if (From.find(".") == string::npos)
        {
            // Int

            // Check size
            long long val = stoll(From);

            if (abs(val) <= __INT_MAX__)
            {
                return Type(atomic, "i32");
            }
            else if (abs(val) <= __LONG_MAX__)
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

typeNode &Type::operator[](const int &Index)
{
    ID = currentID++;
    return internal[Index];
}
