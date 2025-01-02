/*
A manager object which maintains a SQLite3 instance.
Based on work partially funded by Colorado Mesa University.
All functions are inline here.

This software is available via the MIT license, which is
transcribed below. Feel free to simply copy this header into
your project with no acknowledgement or citation needed.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

MIT License

Copyright (c) 2024 Jordan Dehmel

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <sqlite3.h>
#include <stdexcept>
#include <string>
#include <vector>

// The GQL version. This only ever increases with versions.
// This can (and should) be used in static assertions to ensure
// validity.
#define GQL_VERSION 000000005ULL

// Assert compiler version
static_assert(__cplusplus > 201100L, "Invalid C++ std.");

#if (__has_include(<format>) && \
     __cplusplus >= 202000L && \
     !FORCE_CUSTOM_FORMAT)
#include <format>
#define format_str std::format
#else

// Janky version of format, but good enough for our purposes
namespace std
{
inline std::string to_string(const std::string &_what)
{
    return _what;
}
} // namespace std

template <typename... Types>
std::string format_str(const std::string &_format,
                       Types... _args)
{
    const std::vector<std::string> args = {
        std::to_string(_args)...};
    std::vector<std::string> parts;

    // Build here
    uint64_t str_i = 0, arg_i = 0;
    std::string partial;
    partial.reserve(_format.size());

    while (str_i < _format.size())
    {
        if (_format[str_i] == '{')
        {
            if (!partial.empty())
            {
                parts.push_back(partial);
                partial.clear();
            }

            if (str_i + 1 >= _format.size())
            {
                throw std::runtime_error(
                    "Malformed format string.");
            }
            if (arg_i >= args.size())
            {
                throw std::runtime_error(
                    "Malformed format string.");
            }

            ++str_i;

            if (_format[str_i] != '}')
            {
                throw std::runtime_error(
                    "Malformed format string.");
            }

            parts.push_back(args[arg_i]);
            ++arg_i;
        }
        else
        {
            partial.push_back(_format[str_i]);
        }

        ++str_i;
    }
    if (!partial.empty())
    {
        parts.push_back(partial);
    }

    // Construct and return
    uint64_t size = 0;
    for (const auto &item : parts)
    {
        size += item.size();
    }

    std::string out;
    out.reserve(size);

    for (const auto &item : parts)
    {
        out += item;
    }
    return out;
}

#endif

// After this many characters, the query "bounces". This
// ensures that SQLite will not experience parser overflows.
// "Bouncing" replaces the existing query with its ids, leading
// to a less complicated (although not necessarily shorter)
// query.
#ifndef GQL_BOUNCE_THRESH
#define GQL_BOUNCE_THRESH 128
#endif

/*
GQL (Graph SQLite3) manager object

This is a database instance which manages a property graph
$G = (V, E, \lambda, \mu)$, where $V$ is the set of vertices,
$E$ is the set of edges, $\lambda: (E \cup V) \to \Sigma$
associates a label from an arbitrary label set $\Sigma$ to each
edge and vertex, and $\mu: (E \cup V) \times K \to S$ associates
keys from arbitrary key set $K$ to values from arbitrary value
set $S$. In our case, $\Sigma, K$ and $S$ are all `std::string`.
*/
class GQL
{
  public:
    // Internal class definitions
    class Result
    {
      public:
        std::vector<std::vector<std::string>> body;
        std::vector<std::string> headers;

        // Merge rows. Every row in this must have some row in
        // the other for which the ids match.
        inline void merge_rows(const Result &_other)
        {
            const auto other_indices = _other["id"];
            std::map<int, int> to_other;
            for (uint i = 0; i < size(); ++i)
            {
                to_other[i] = std::distance(
                    other_indices.begin(),
                    std::find(other_indices.begin(),
                              other_indices.end(),
                              operator[]("id")[i]));
            }

            // For each column in other which is not in this
            for (const auto &col : _other.headers)
            {
                if (std::find(headers.begin(), headers.end(),
                              col) != headers.end())
                {
                    continue;
                }

                // Append header
                const auto col_vals = _other[col];
                headers.push_back(col);
                for (uint i = 0; i < size(); ++i)
                {
                    body[i].push_back(col_vals.at(to_other[i]));
                }
            }
        }

        // Get row
        inline const std::vector<std::string> &operator[](
            const size_t &_i) const
        {
            return body[_i];
        }

        // Get column (using other helper functions)
        inline const std::vector<std::string> operator[](
            const std::string &_col) const
        {
            std::vector<std::string> out;
            out.reserve(size());
            const auto ind = index_of(_col);

            for (uint i = 0; i < size(); ++i)
            {
                out.push_back(operator[](i)[ind]);
            }

            return out;
        }

        // Iterator functions
        std::vector<std::vector<std::string>>::const_iterator
        begin() const
        {
            return body.begin();
        }

        std::vector<std::vector<std::string>>::const_iterator
        end() const
        {
            return body.end();
        }

        // Get the number of items in this result
        inline size_t size() const noexcept
        {
            return body.size();
        }

        // Returns true iff the result is empty
        inline bool empty() const noexcept
        {
            return body.empty();
        }

        // Returns the index of the given column (given that it
        // exists)
        inline size_t index_of(const std::string &_what) const
        {
            auto it = std::find(headers.begin(), headers.end(),
                                _what);
            if (it == headers.end())
            {
                throw std::runtime_error(format_str(
                    "Header value '{}' is not present in "
                    "results.",
                    _what));
            }
            return std::distance(headers.begin(), it);
        }
    };

    class Edges;

    class Vertices
    {
      protected:
        GQL *const owner;
        std::string cmd;

      public:
        // Bounces may happen herein
        Vertices(GQL *const _owner, const std::string &_cmd);

        Vertices &operator=(const Vertices &_other)
        {
            assert(owner == _other.owner);
            cmd = _other.cmd;
            return *this;
        }

        // Select all vertices where the given SQL statement
        // holds
        Vertices where(const std::string &_sql_statement);

        // Limit to some number
        Vertices limit(const uint64_t &_n);

        // Select all vertices with the given label
        Vertices with_label(const std::string &_label);

        // Select all vertices where the given tag key is
        // associated with the given value
        Vertices with_tag(const std::string &_key,
                          const std::string &_value);

        // Select all vertices with the given id
        Vertices with_id(const uint64_t &_id);

        // Select all vertices which are in this, the passed
        // set, or both
        Vertices join(const Vertices &_other);

        // Select all vertices which are in both this and the
        // passed set
        Vertices intersection(const Vertices &_other);

        // Select all vertices which are in the universe set but
        // not this set
        Vertices complement(const Vertices &_universe);

        // Select all vertices which are in this set but not the
        // subgroup
        Vertices excluding(const Vertices &_subgroup);

        // Select all nodes reachable from this node set
        // according to a few recursive rules. This follows
        // edges FORWARDS. A new node is visited iff:
        // 1)   it is the target of an edge for which
        //      _where_edge holds and source is already in the
        //      traversal
        // 2)   _where_node holds for it
        Vertices traverse(const std::string &_where_node = "1",
                          const std::string &_where_edge = "1");

        // Select all nodes reachable from this node set
        // according to a few recursive rules. This follows
        // edges BACKWARDS. A new node is visited iff:
        // 1)   it is the source of an edge for which
        //      _where_edge holds and target is already in the
        //      traversal
        // 2)   _where_node holds for it
        Vertices r_traverse(
            const std::string &_where_node = "1",
            const std::string &_where_edge = "1");

        // Select the label from all nodes in this set
        Result label();

        // Select the value associated with the given key for
        // all vertices in this set
        Result tag(const std::string &_key);
        Result tag(const std::list<std::string> &_keys);

        // Select the id of all vertices in this set
        Result id();

        // Return the results of the SQL `SELECT {} FROM (...)`,
        // where `{}` is the passed string and `...` is this set
        Result select(const std::string &_sql = "*");

        // Set the label associated with all the nodes herein
        Vertices label(const std::string &_label);

        // For each vertex in this set, associate the tag `_key`
        // with the value `_value`
        Vertices tag(const std::string &_key,
                     const std::string &_value);

        // Erase these nodes from the database. This will also
        // erase any and all edges which reference the deleted
        // nodes as either source or target.
        void erase();

        // Run some function given this set, then continue on.
        // For instance,
        // ```cpp
        // a.lemma([](auto self){
        //    self.tag("foo", "fizz"); }
        // ).erase();
        // ```
        // would execute a tag, then erase on the same node.
        Vertices lemma(const std::function<void(Vertices)> _fn);

        // Add edges from each node in this set to each node
        // in the passed set (cartesian product).
        Edges add_edge(const Vertices &_to);

        // Get all edges leading into these vertices
        Edges in();

        // Get all edges leading out of these vertices
        Edges out();

        // Gets only the vertices with the given in degree
        Vertices with_in_degree(
            const uint64_t &_count,
            const std::string &_where_edge = "1");

        // Gets only the vertices with the given out degree
        Vertices with_out_degree(
            const uint64_t &_count,
            const std::string &_where_edge = "1");

        // Gets the in-degrees of the given nodes
        Result in_degree(const std::string &_where_edge = "1");

        // Gets the out-degrees of the given nodes
        Result out_degree(const std::string &_where_edge = "1");

        friend class Edges;
    };

    class Edges
    {
      protected:
        GQL *const owner;
        std::string cmd;

      public:
        // Bounces may occur herein
        Edges(GQL *const _owner, const std::string &_cmd);

        Edges &operator=(const Edges &_other)
        {
            assert(owner == _other.owner);
            cmd = _other.cmd;
            return *this;
        }

        // Select some subset of these edges according to a SQL
        // condition
        Edges where(const std::string &_sql_statement);

        // Limit to some number
        Edges limit(const uint64_t &_n);

        // Select all edges with their source in the given set
        Edges with_source(const GQL::Vertices &_source);

        // Select all edges with their target in the given set
        Edges with_target(const GQL::Vertices &_source);

        // Select all edges which have the given label
        Edges with_label(const std::string &_label);

        // Get all edges which have the given key-value pair
        Edges with_tag(const std::string &_key,
                       const std::string &_value);

        // Get all edges which have the given id
        Edges with_id(const uint64_t &_id);

        // Return all values which are in either this set or
        // the passed one or both
        Edges join(const Edges &_other);

        // Return all values which are in both this set and the
        // passed one
        Edges intersection(const Edges &_other);

        // Return all values which are in the passed set but not
        // in this set
        Edges complement(const Edges &_universe);

        // Return all values which are in this set but not in
        // the subgroup
        Edges excluding(const Edges &_subgroup);

        // Get the label of all edges in this set
        Result label();

        // Get the values associated with the given key
        Result tag(const std::string &_key);
        Result tag(const std::list<std::string> &_keys);

        // Yield the ids of the edges in this set
        Result id();

        // Select something from these edges.
        // The form is: `SELECT {} FROM (...)`, with your
        // statement replacing `{}`.
        Result select(const std::string &_sql = "*");

        // Set the label of these edges
        Edges label(const std::string &_label);

        // Add a key-value pair to each of these edges.
        Edges tag(const std::string &_key,
                  const std::string &_value);

        // Run some function given this set, then continue on.
        // For instance,
        // ```cpp
        // a.lemma([](auto self){
        //    self.tag("foo", "fizz"); }
        // ).erase();
        // ```
        // would execute a tag, then erase on the same node.
        Edges lemma(const std::function<void(Edges)> _fn);

        // Erase these edges from the database.
        void erase();

        // Yield all vertices which are sources in this set of
        // edges.
        Vertices source();

        // Yield all vertices which are targets in this set of
        // edges.
        Vertices target();
    };

    /*
    Initialize from a file, or create it if it DNE. If _erase,
    purges all nodes and edges.
    */
    GQL(const std::string &_filepath = "gql.db",
        const bool &_erase = false);

    // Closes the database
    ~GQL();

    // Get all vertices
    Vertices v();

    // Get all edges
    Edges e();

    // Get all vertices where some SQL WHERE clause holds
    Vertices v(const std::string &_where);

    // Get all edges where some SQL WHERE clause holds
    Edges e(const std::string &_where);

    // Save a graphviz (.dot) representation of the database
    void graphviz(const std::string &_filepath);

    // Commit the database and open a new transaction.
    void commit();

    // Revert this transaction
    void rollback();

    // Generate a new vertex and return a handle to it
    Vertices add_vertex();

    // Generate a new vertex with the given id and return a
    // handle to it
    Vertices add_vertex(const uint64_t &_id);

    // Generate an edge from the given source to the given
    // target
    Edges add_edge(const uint64_t &_source,
                   const uint64_t &_target);

    // Increments with each SQL query submitted
    uint64_t sql_call_counter = 0;

  protected:
    // Execute the given sql and return the results
    Result sql(const std::string &_sql);

    sqlite3 *db = nullptr;
    uint64_t next_node_id = 1, next_edge_id = 1;
};

std::ostream &operator<<(std::ostream &_into,
                         const GQL::Result &_what);

////////////////////////////////////////////////////////////////

inline GQL::GQL(const std::string &_filepath,
                const bool &_erase)
{
    // Open database
    sqlite3_open(_filepath.c_str(), &db);

    // Initialize
    sql("CREATE TABLE IF NOT EXISTS nodes ("
        "id INTEGER NOT NULL, "
        "label TEXT DEFAULT '', "
        "tags TEXT DEFAULT '{}', "
        "PRIMARY KEY(id)"
        ");");
    sql("CREATE TABLE IF NOT EXISTS edges ("
        "id INTEGER NOT NULL, "
        "source INTEGER NOT NULL, "
        "target INTEGER NOT NULL, "
        "label TEXT DEFAULT '', "
        "tags TEXT DEFAULT '{}', "
        "PRIMARY KEY(id), "
        "FOREIGN KEY(source) REFERENCES nodes(id), "
        "FOREIGN KEY(target) REFERENCES nodes(id)"
        ");");

    // Indices here
    sql("CREATE INDEX IF NOT EXISTS edge_src "
        "ON edges(source);");
    sql("CREATE INDEX IF NOT EXISTS edge_tgt "
        "ON edges(target);");
    sql("CREATE INDEX IF NOT EXISTS edge_lbl "
        "ON edges(label);");
    sql("CREATE INDEX IF NOT EXISTS edge_id "
        "ON edges(label);");
    sql("CREATE INDEX IF NOT EXISTS node_label "
        "ON nodes(label);");
    sql("CREATE INDEX IF NOT EXISTS node_id "
        "ON nodes(id);");

    if (_erase)
    {
        sql("DELETE FROM nodes;");
        sql("DELETE FROM edges;");
    }

    sql("BEGIN;");
}

inline GQL::~GQL()
{
    sql("COMMIT;");
    sqlite3_close(db);
}

inline void GQL::commit()
{
    sql("COMMIT;");
    sql("BEGIN;");
}

inline void GQL::rollback()
{
    sql("ROLLBACK;");
    sql("BEGIN;");
}

////////////////////////////////////////////////////////////////

inline GQL::Vertices::Vertices(GQL *const _owner,
                               const std::string &_cmd)
    : owner(_owner), cmd(_cmd)
{
    if (_cmd.size() > GQL_BOUNCE_THRESH)
    {
        // Perform query simplification "bounce"
        auto ids = this->id();

        std::string bounced_cmd =
            "SELECT * FROM nodes WHERE id IN (";
        if (ids.empty())
        {
            bounced_cmd += ')';
        }
        for (const auto &id : ids)
        {
            bounced_cmd += id[0] + ",";
        }
        bounced_cmd.back() = ')';

        cmd = bounced_cmd;
    }
}

inline GQL::Vertices GQL::Vertices::where(
    const std::string &_sql_statement)
{
    return GQL::Vertices(
        owner, format_str("SELECT * FROM ({}) WHERE {}", cmd,
                          _sql_statement));
}

inline GQL::Vertices GQL::Vertices::limit(const uint64_t &_n)
{
    return GQL::Vertices(
        owner,
        format_str("SELECT * FROM ({}) LIMIT {}", cmd, _n));
}

inline GQL::Vertices GQL::Vertices::with_label(
    const std::string &_label)
{
    return GQL::Vertices(owner,
                         format_str("SELECT * FROM ({}) WHERE "
                                    "label = '{}'",
                                    cmd, _label));
}

inline GQL::Vertices GQL::Vertices::with_tag(
    const std::string &_key, const std::string &_value)
{
    return GQL::Vertices(
        owner, format_str("SELECT * FROM ({}) WHERE "
                          "json_extract(tags, '$.{}') = '{}'",
                          cmd, _key, _value));
}

inline GQL::Vertices GQL::Vertices::with_id(const uint64_t &_id)
{
    return GQL::Vertices(owner,
                         format_str("SELECT * FROM ({}) WHERE "
                                    "id = {}",
                                    cmd, _id));
}

inline GQL::Vertices GQL::Vertices::join(
    const GQL::Vertices &_with)
{
    return GQL::Vertices(
        owner, format_str("{} UNION {}", cmd, _with.cmd));
}

inline GQL::Vertices GQL::Vertices::intersection(
    const GQL::Vertices &_with)
{
    return GQL::Vertices(
        owner, format_str("{} INTERSECT {}", cmd, _with.cmd));
}

inline GQL::Vertices GQL::Vertices::complement(
    const GQL::Vertices &_universe)
{
    return GQL::Vertices(
        owner, format_str("SELECT * FROM ({}) WHERE id NOT IN "
                          "(SELECT id FROM ({}))",
                          _universe.cmd, cmd));
}

// Select all vertices which are in this set but not the
// subgroup
inline GQL::Vertices GQL::Vertices::excluding(
    const GQL::Vertices &_subgroup)
{
    return GQL::Vertices(
        owner, format_str("SELECT * FROM ({}) WHERE id NOT IN "
                          "(SELECT id FROM ({}))",
                          cmd, _subgroup.cmd));
}

inline GQL::Vertices GQL::Vertices::traverse(
    const std::string &_where_node,
    const std::string &_where_edge)
{
    return GQL::Vertices(
        owner,
        format_str("WITH RECURSIVE t AS ("
                   // Base case
                   "SELECT id FROM ({}) "
                   // Recursive call
                   "UNION "
                   "SELECT e.target AS id FROM t "
                   "JOIN (SELECT * FROM edges WHERE {}) e "
                   "ON t.id = e.source "
                   "JOIN (SELECT * FROM nodes WHERE {}) n "
                   "ON e.target = n.id "
                   "WHERE e.target IS NOT NULL) "
                   // End recursive call
                   "SELECT * FROM t JOIN nodes "
                   "ON t.id = nodes.id",
                   cmd, _where_edge, _where_node));
}

inline GQL::Vertices GQL::Vertices::r_traverse(
    const std::string &_where_node,
    const std::string &_where_edge)
{
    return GQL::Vertices(
        owner,
        format_str("WITH RECURSIVE t AS ("
                   // Base case
                   "SELECT id FROM ({}) "
                   // Recursive call
                   "UNION "
                   "SELECT e.source AS id "
                   "FROM t "
                   "JOIN (SELECT * FROM edges WHERE {}) e "
                   "ON t.id = e.target "
                   "JOIN (SELECT * FROM nodes WHERE {}) n "
                   "ON e.source = n.id "
                   "WHERE e.source IS NOT NULL) "
                   // End recursive call
                   "SELECT * FROM t JOIN nodes "
                   "ON t.id = nodes.id",
                   cmd, _where_edge, _where_node));
}

inline GQL::Result GQL::Vertices::label()
{
    return owner->sql(
        format_str("SELECT id, label AS label FROM "
                   "({}) ORDER BY id;",
                   cmd));
}

inline GQL::Result GQL::Vertices::tag(const std::string &_key)
{
    return owner->sql(
        format_str("SELECT id, "
                   "json_extract(tags, '$.{}') AS {} "
                   "FROM ({}) ORDER BY id;",
                   _key, _key, cmd));
}

inline GQL::Result GQL::Vertices::tag(
    const std::list<std::string> &_keys)
{
    std::string subcommand = "";
    for (const auto &key : _keys)
    {
        if (!subcommand.empty())
        {
            subcommand += ", ";
        }

        if (key == "id" || key == "tags" || key == "label")
        {
            subcommand += key;
        }
        else
        {
            subcommand += format_str(
                "json_extract(tags, '$.{}') AS {}", key, key);
        }
    }

    return owner->sql(format_str(
        "SELECT {} FROM ({}) ORDER BY id;", subcommand, cmd));
}

inline GQL::Result GQL::Vertices::id()
{
    return owner->sql(
        format_str("SELECT id FROM ({}) ORDER BY id;", cmd));
}

inline GQL::Result GQL::Vertices::select(
    const std::string &_what)
{
    return owner->sql(format_str(
        "SELECT id, {} FROM ({}) ORDER BY id;", _what, cmd));
}

inline GQL::Vertices GQL::Vertices::label(
    const std::string &_label)
{
    owner->sql(
        format_str("UPDATE nodes SET label = '{}' WHERE id IN "
                   "(SELECT id FROM ({}))",
                   _label, cmd));

    return *this;
}

inline GQL::Vertices GQL::Vertices::tag(
    const std::string &_key, const std::string &_value)
{
    owner->sql(format_str(
        "UPDATE nodes SET tags = json_set(tags, '$.{}', '{}') "
        "WHERE id IN (SELECT id FROM ({}))",
        _key, _value, cmd));

    return *this;
}

inline GQL::Vertices GQL::Vertices::lemma(
    const std::function<void(GQL::Vertices)> _fn)
{
    _fn(*this);
    return *this;
}

inline void GQL::Vertices::erase()
{
    owner->sql(format_str("DELETE FROM nodes WHERE id IN "
                          "(SELECT id FROM ({}));",
                          cmd));

    // Erase dead edges
    owner->sql("WITH ids AS (SELECT id FROM nodes) "
               "DELETE FROM edges WHERE "
               "source NOT IN ids "
               "OR target NOT IN ids;");
}

inline GQL::Edges GQL::Vertices::in()
{
    return GQL::Edges(
        owner, format_str("SELECT * FROM edges WHERE target IN "
                          "(SELECT id FROM ({}))",
                          cmd));
}

inline GQL::Edges GQL::Vertices::out()
{
    return GQL::Edges(
        owner, format_str("SELECT * FROM edges WHERE source IN "
                          "(SELECT id FROM ({}))",
                          cmd));
}

inline GQL::Vertices GQL::Vertices::with_in_degree(
    const uint64_t &_count, const std::string &_where_edge)
{
    return GQL::Vertices(
        owner,
        format_str(
            "WITH n AS ({}) "
            "SELECT id, label, tags FROM ("
            "SELECT n.*, COUNT(e.id) AS c "
            "FROM n LEFT JOIN (SELECT * FROM edges WHERE {}) e "
            "ON e.target = n.id "
            "GROUP BY n.id) t "
            "WHERE t.c = {}",
            cmd, _where_edge, _count));
}

inline GQL::Vertices GQL::Vertices::with_out_degree(
    const uint64_t &_count, const std::string &_where_edge)
{
    return GQL::Vertices(
        owner,
        format_str(
            "WITH n AS ({}) "
            "SELECT id, label, tags FROM ("
            "SELECT n.*, COUNT(e.id) AS c "
            "FROM n LEFT JOIN (SELECT * FROM edges WHERE {}) e "
            "ON e.source = n.id "
            "GROUP BY n.id) t "
            "WHERE t.c = {}",
            cmd, _where_edge, _count));
}

// Gets the in-degrees of the given nodes
inline GQL::Result GQL::Vertices::in_degree(
    const std::string &_where_edge)
{
    return owner->sql(format_str(
        "WITH n AS ({}) "
        "SELECT t.id AS id, t.c AS in_degree FROM ("
        "SELECT n.id AS id, COUNT(e.id) AS c "
        "FROM n LEFT JOIN (SELECT * FROM edges WHERE {}) e "
        "ON e.target = n.id "
        "GROUP BY n.id) t "
        "ORDER BY id;",
        cmd, _where_edge));
}

// Gets the out-degrees of the given nodes
inline GQL::Result GQL::Vertices::out_degree(
    const std::string &_where_edge)
{
    return owner->sql(format_str(
        "WITH n AS ({}) "
        "SELECT t.id AS id, t.c AS out_degree FROM ("
        "SELECT n.id AS id, COUNT(e.id) AS c "
        "FROM n LEFT JOIN (SELECT * FROM edges WHERE {}) e "
        "ON e.source = n.id "
        "GROUP BY n.id) t "
        "ORDER BY id;",
        cmd, _where_edge));
}

inline GQL::Edges GQL::Vertices::add_edge(
    const GQL::Vertices &_other)
{
    owner->sql("INSERT INTO edges (source, target) "
               "SELECT l.id, r.id " +
               format_str("FROM ({}) l CROSS JOIN ({}) r", cmd,
                          _other.cmd));

    return GQL::Edges(owner,
                      format_str("SELECT * FROM edges "
                                 "WHERE (source, target) IN "
                                 "(SELECT l.id, r.id FROM "
                                 "({}) l CROSS JOIN ({}) r)",
                                 cmd, _other.cmd));
}

////////////////////////////////////////////////////////////////

inline GQL::Edges::Edges(GQL *const _owner,
                         const std::string &_cmd)
    : owner(_owner), cmd(_cmd)
{
    if (_cmd.size() > GQL_BOUNCE_THRESH)
    {
        // Perform query simplification "bounce"
        auto ids = this->id();

        std::string bounced_cmd =
            "SELECT * FROM edges WHERE id IN (";
        if (ids.empty())
        {
            bounced_cmd += ')';
        }
        for (const auto &id : ids)
        {
            bounced_cmd += id[0] + ",";
        }
        bounced_cmd.back() = ')';

        cmd = bounced_cmd;
    }
}

inline GQL::Edges GQL::Edges::where(
    const std::string &_sql_statement)
{
    return GQL::Edges(owner,
                      format_str("SELECT * FROM ({}) WHERE {}",
                                 cmd, _sql_statement));
}

inline GQL::Edges GQL::Edges::limit(const uint64_t &_n)
{
    return GQL::Edges(
        owner,
        format_str("SELECT * FROM ({}) LIMIT {}", cmd, _n));
}

inline GQL::Edges GQL::Edges::with_source(
    const GQL::Vertices &_source)
{
    return GQL::Edges(
        owner, format_str("SELECT * FROM ({}) WHERE "
                          "source IN (SELECT id FROM ({}))",
                          cmd, _source.cmd));
}

inline GQL::Edges GQL::Edges::with_target(
    const GQL::Vertices &_source)
{
    return GQL::Edges(
        owner, format_str("SELECT * FROM ({}) WHERE "
                          "target IN (SELECT id FROM ({}))",
                          cmd, _source.cmd));
}

inline GQL::Edges GQL::Edges::with_label(
    const std::string &_label)
{
    return GQL::Edges(owner,
                      format_str("SELECT * FROM ({}) WHERE "
                                 "label = '{}'",
                                 cmd, _label));
}

inline GQL::Edges GQL::Edges::with_tag(
    const std::string &_key, const std::string &_value)
{
    return GQL::Edges(
        owner, format_str("SELECT * FROM ({}) WHERE "
                          "json_extract(tags, '$.{}') = '{}'",
                          cmd, _key, _value));
}

// Get all edges which have the given id
inline GQL::Edges GQL::Edges::with_id(const uint64_t &_id)
{
    return GQL::Edges(owner,
                      format_str("SELECT * FROM ({}) WHERE "
                                 "id = {}",
                                 cmd, _id));
}

inline GQL::Edges GQL::Edges::join(const GQL::Edges &_with)
{
    return GQL::Edges(
        owner, format_str("{} UNION {}", cmd, _with.cmd));
}

inline GQL::Edges GQL::Edges::intersection(
    const GQL::Edges &_with)
{
    return GQL::Edges(
        owner, format_str("{} INTERSECT {}", cmd, _with.cmd));
}

inline GQL::Edges GQL::Edges::complement(
    const GQL::Edges &_universe)
{
    return GQL::Edges(
        owner, format_str("SELECT * FROM ({}) WHERE id NOT IN "
                          "(SELECT id FROM ({}))",
                          _universe.cmd, cmd));
}

// Select all vertices which are in this set but not the
// subgroup
inline GQL::Edges GQL::Edges::excluding(
    const GQL::Edges &_subgroup)
{
    return GQL::Edges(
        owner, format_str("SELECT * FROM ({}) WHERE id NOT IN "
                          "(SELECT id FROM ({}))",
                          cmd, _subgroup.cmd));
}

inline GQL::Result GQL::Edges::label()
{
    return owner->sql(format_str(
        "SELECT id, label FROM ({}) ORDER BY id;", cmd));
}

inline GQL::Result GQL::Edges::tag(const std::string &_key)
{
    return owner->sql(format_str("SELECT id, "
                                 "json_extract(tags, '$.{}') "
                                 "AS {} FROM ({}) ORDER BY id;",
                                 _key, _key, cmd));
}

inline GQL::Result GQL::Edges::tag(
    const std::list<std::string> &_keys)
{
    std::string subcommand = "";
    for (const auto &key : _keys)
    {
        if (!subcommand.empty())
        {
            subcommand += ", ";
        }

        if (key == "id" || key == "tags" || key == "label" ||
            key == "source" || key == "target")
        {
            subcommand += key;
        }
        else
        {
            subcommand += format_str(
                "json_extract(tags, '$.{}') AS {}", key, key);
        }
    }

    return owner->sql(format_str(
        "SELECT {} FROM ({}) ORDER BY id;", subcommand, cmd));
}

inline GQL::Result GQL::Edges::id()
{
    return owner->sql(
        format_str("SELECT id FROM ({}) ORDER BY id;", cmd));
}

inline GQL::Result GQL::Edges::select(const std::string &_what)
{
    return owner->sql(format_str(
        "SELECT id, {} FROM ({}) ORDER BY id;", _what, cmd));
}

inline GQL::Edges GQL::Edges::label(const std::string &_label)
{
    owner->sql(
        format_str("UPDATE edges SET label = '{}' WHERE id IN "
                   "(SELECT id FROM ({}))",
                   _label, cmd));

    return *this;
}

inline GQL::Edges GQL::Edges::tag(const std::string &_key,
                                  const std::string &_value)
{
    owner->sql(format_str(
        "UPDATE edges SET tags = json_set(tags, '$.{}', '{}') "
        "WHERE id IN (SELECT id FROM ({}))",
        _key, _value, cmd));

    return *this;
}

inline GQL::Edges GQL::Edges::lemma(
    const std::function<void(GQL::Edges)> _fn)
{
    _fn(*this);
    return *this;
}

inline void GQL::Edges::erase()
{
    owner->sql(format_str("DELETE FROM edges WHERE id IN "
                          "(SELECT id FROM ({}))",
                          cmd));
}

inline GQL::Vertices GQL::Edges::source()
{
    return GQL::Vertices(
        owner, format_str("SELECT * FROM nodes WHERE id IN "
                          "(SELECT source AS id FROM ({}))",
                          cmd));
}

inline GQL::Vertices GQL::Edges::target()
{
    return GQL::Vertices(
        owner, format_str("SELECT * FROM nodes WHERE id IN "
                          "(SELECT target AS id FROM ({}))",
                          cmd));
}

////////////////////////////////////////////////////////////////

inline GQL::Vertices GQL::v()
{
    return GQL::Vertices(this, "SELECT * FROM nodes");
}

inline GQL::Edges GQL::e()
{
    return GQL::Edges(this, "SELECT * FROM edges");
}

inline GQL::Vertices GQL::v(const std::string &_where)
{
    return GQL::Vertices(this,
                         "SELECT * FROM nodes WHERE " + _where);
}

inline GQL::Edges GQL::e(const std::string &_where)
{
    return GQL::Edges(this,
                      "SELECT * FROM edges WHERE " + _where);
}

inline GQL::Vertices GQL::add_vertex()
{
    // Add vertex
    const auto id = next_node_id++;
    sql(format_str("INSERT INTO nodes (id, tags) VALUES ({}, ",
                   id) +
        "json('{}'));");

    // Return query
    return v(format_str("id = {}", id));
}

inline GQL::Vertices GQL::add_vertex(const uint64_t &_id)
{
    // Add vertex
    sql(format_str("INSERT INTO nodes (id, tags) VALUES ({}, ",
                   _id) +
        "json('{}'));");

    // Return query
    return v(format_str("id = {}", _id));
}

inline GQL::Edges GQL::add_edge(const uint64_t &_source,
                                const uint64_t &_target)
{
    // Add edge
    const auto id = next_edge_id++;
    sql(format_str(
            "INSERT INTO edges (id, source, target, tags) "
            "VALUES ({}, {}, {}, ",
            id, _source, _target) +
        "json('{}'));");

    // Return query
    return e(format_str("id = {}", id));
}

////////////////////////////////////////////////////////////////

inline GQL::Result GQL::sql(const std::string &_stmt)
{
    ++sql_call_counter;

    char *err_msg = nullptr;
    const static auto callback = [](void *out, int n_cols,
                                    char **col_vals,
                                    char **col_names) {
        GQL::Result *q = (GQL::Result *)out;

        if (q->headers.empty())
        {
            for (int i = 0; i < n_cols; ++i)
            {
                if (col_names[i] == nullptr)
                {
                    q->headers.push_back("NULL");
                }
                else
                {
                    q->headers.push_back(col_names[i]);
                }
            }
        }

        std::vector<std::string> row;
        for (int i = 0; i < n_cols; ++i)
        {
            if (col_vals[i] == nullptr)
            {
                row.push_back("NULL");
            }
            else
            {
                row.push_back(col_vals[i]);
            }
        }
        q->body.push_back(row);

        return 0;
    };

    GQL::Result out;
    auto res = sqlite3_exec(db, _stmt.c_str(), callback, &out,
                            &err_msg);

    if (err_msg != NULL || res != SQLITE_OK)
    {
        std::cerr << "In SQL '" << _stmt << "':\n";
        throw std::runtime_error(err_msg);
    }

    return out;
}

inline void GQL::graphviz(const std::string &_filepath)
{
    auto sanitize = [](std::string &_w) {
        for (uint64_t i = 0; i < _w.size(); ++i)
        {
            switch (_w[i])
            {
            case '"':
                _w.replace(i, 1, "\\\"");
            case '\\':
                ++i;
            }
        }
    };

    // Open + header
    std::ofstream f(_filepath);
    if (!f.is_open())
    {
        throw std::runtime_error(format_str(
            "Failed to open output graphviz file '{}'.",
            _filepath));
    }
    f << "digraph {\n\tforcelabels=true;\n";

    // Nodes
    auto node_data = v().select("id, label, tags");

    if (!node_data.body.empty())
    {
        auto id_i = node_data.index_of("id");
        auto label_i = node_data.index_of("label");
        auto tags_i = node_data.index_of("tags");

        for (const auto &node : node_data.body)
        {
            auto id = node[id_i];
            auto label = node[label_i];
            auto tags = node[tags_i];

            sanitize(label);
            sanitize(tags);

            if (tags == "{}")
            {
                tags = "";
            }

            f << '\t' << id << " [label=\"" << label
              << "\", xlabel=\"" << tags << "\"];\n";
        }
    }

    // Edges
    auto edge_data = e().select("source, target, label, tags");

    if (!edge_data.body.empty())
    {
        auto source_i = edge_data.index_of("source");
        auto target_i = edge_data.index_of("target");
        auto label_i = edge_data.index_of("label");
        auto tags_i = edge_data.index_of("tags");

        for (const auto &edge : edge_data.body)
        {
            auto source = edge[source_i];
            auto target = edge[target_i];
            auto label = edge[label_i];
            auto tags = edge[tags_i];

            sanitize(label);
            sanitize(tags);

            if (tags == "{}")
            {
                tags = "";
            }

            f << '\t' << source << " -> " << target
              << " [label=\"" << label << "\", xlabel=\""
              << tags << "\"];\n";
        }
    }

    // Footer + close
    f << "}\n";
    f.close();
}

inline std::ostream &operator<<(std::ostream &_strm,
                                const GQL::Result &_res)
{
    // Headers
    for (uint64_t i = 0; i < _res.headers.size(); ++i)
    {
        if (i != 0)
        {
            _strm << '|';
        }
        _strm << _res.headers[i];
    }
    _strm << '\n';

    // Body
    for (uint64_t row = 0; row < _res.body.size(); ++row)
    {
        for (uint64_t i = 0; i < _res.headers.size(); ++i)
        {
            if (i != 0)
            {
                _strm << '|';
            }
            _strm << _res.body[row][i];
        }
        _strm << '\n';
    }

    return _strm;
}

////////////////////////////////////////////////////////////////
#if (__has_include(<format>) && !FORCE_CUSTOM_FORMAT)
#undef format_str
#endif
