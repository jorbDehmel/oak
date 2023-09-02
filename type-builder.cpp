#include "type-builder.hpp"

set<string> deps;
map<string, __structLookupData> structData;
vector<string> structOrder;

const Type nullType(atomic, "NULL");

Type::Type(const TypeInfo &Info, const string &Name)
{
    info = Info;
    name = Name;
    next = nullptr;

    return;
}

Type::Type(const Type &What)
{
    info = What.info;
    name = What.name;

    if (What.next == nullptr)
    {
        next = nullptr;
    }
    else
    {
        next = new Type(*What.next);
    }

    return;
}

Type::Type()
{
    info = atomic;
    name = "NULL";
    next = nullptr;

    return;
}

Type::~Type()
{
    if (next != nullptr)
    {
        delete next;
    }

    return;
}

void Type::prepend(const TypeInfo &Info, const string &Name)
{
    Type *temp = new Type(info, name);
    temp->next = next;

    next = temp;

    info = Info;
    name = Name;

    return;
}

void Type::append(const TypeInfo &Info, const string &Name)
{
    if (info == atomic && name == "NULL" && next == nullptr)
    {
        info = Info;
        name = Name;
    }
    else if (next == nullptr)
    {
        next = new Type(Info, Name);
    }
    else
    {
        next->append(Info, Name);
    }

    return;
}

void Type::append(const Type &Other)
{
    if (info == atomic && name == "NULL" && next == nullptr)
    {
        info = Other.info;
        name = Other.name;

        next = new Type(*Other.next);
    }
    else if (next == nullptr)
    {
        next = new Type(Other);
    }
    else
    {
        next->append(Other);
    }
}

bool Type::operator==(const Type &Other) const
{
    if (info != Other.info)
    {
        return false;
    }
    else if ((info == atomic || info == modifier) && name != Other.name)
    {
        return false;
    }
    else if (next == nullptr)
    {
        return Other.next == nullptr;
    }

    return *next == *Other.next;
}

bool Type::operator!=(const Type &Other) const
{
    return !(*this == Other);
}

Type &Type::operator=(const Type &Other)
{
    if (next != nullptr)
    {
        delete next;
    }

    info = Other.info;
    name = Other.name;

    if (Other.next == nullptr)
    {
        next = nullptr;
    }
    else
    {
        next = new Type(*Other.next);
    }

    return *this;
}

string toStr(const Type *const What)
{
    if (What == nullptr)
    {
        return "";
    }

    string out = "";

    switch (What->info)
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
        out += What->name + ":";
        break;
    default:
        out += What->name;
        break;
    }

    if (What->next != nullptr)
    {
        if (What->next->info != function && What->next->info != pointer &&
            What->next->info != join && What->next->info != maps &&
            What->info != pointer && What->info != function)
        {
            out += " ";
        }
        else if (What->info == var_name)
        {
            out += " ";
        }

        out += toStr(What->next);
    }

    return out;
}

// More efficient than Type::pop_front()
void popTypeFront(Type &What)
{
    if (What.next != nullptr)
    {
        auto oldFront = What;
        What = *oldFront.next;
    }
    else
    {
        What = nullType;
    }

    return;
}

void Type::pop_front()
{
    if (next == nullptr)
    {
        info = atomic;
        name = "NULL";
    }
    else
    {
        *this = *next;
    }

    return;
}

void Type::pop_back()
{
    if (next == nullptr)
    {
        info = atomic;
        name = "NULL";
    }
    else if (next->next == nullptr)
    {
        delete next;
        next = nullptr;
    }
    else
    {
        next->pop_back();
    }

    return;
}
