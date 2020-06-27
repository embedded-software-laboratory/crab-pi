#pragma once

/* Liveness analysis */

//#include <crab/cfg/basic_block_traits.hpp>
#include <crab/domains/killgen_domain.hpp>
#include <crab/iterators/killgen_fixpoint_iterator.hpp>
#include <crab/support/debug.hpp>
#include <crab/support/stats.hpp>

#include <boost/range/iterator_range.hpp>
#include <unordered_map>

namespace crab {

namespace analyzer {

template <typename V>
using varset_domain = crab::domains::flat_killgen_domain<V>;

/**
 * Define the main operations for the liveness variable analysis:
 * compute for each basic block the set of live variables, i.e.,
 * variables that might be used in the future
 **/
template <class CFG>
class liveness_analysis_operations
    : public crab::iterators::killgen_operations_api<
          CFG, varset_domain<typename CFG::variable_t>> {

public:
  using varset_domain_t = varset_domain<typename CFG::variable_t>;
  using basic_block_label_t = typename CFG::basic_block_label_t;

private:
  using parent_type =
      crab::iterators::killgen_operations_api<CFG, varset_domain_t>;
  using binding_t = std::pair<varset_domain_t, varset_domain_t>;
  using liveness_map_t = std::unordered_map<basic_block_label_t, binding_t>;

  liveness_map_t m_liveness_map;

public:
  liveness_analysis_operations(CFG cfg) : parent_type(cfg) {}

  virtual bool is_forward() { return false; }

  virtual varset_domain_t entry() {
    varset_domain_t res = varset_domain_t::bottom();
    if (this->m_cfg.has_func_decl()) {
      auto fdecl = this->m_cfg.get_func_decl();
      for (unsigned i = 0, e = fdecl.get_num_outputs(); i < e; ++i) {
        res += fdecl.get_output_name(i);
      }
    }
    return res;
  }

  virtual varset_domain_t merge(varset_domain_t d1, varset_domain_t d2) {
    return d1 | d2;
  }

  virtual void init_fixpoint() {
    for (auto &b :
         boost::make_iterator_range(this->m_cfg.begin(), this->m_cfg.end())) {
      varset_domain_t kill, gen;
      for (auto &s : boost::make_iterator_range(b.rbegin(), b.rend())) {
        auto const &live = s.get_live();
        for (auto d :
             boost::make_iterator_range(live.defs_begin(), live.defs_end())) {
          kill += d;
          gen -= d;
        }
        for (auto u :
             boost::make_iterator_range(live.uses_begin(), live.uses_end())) {
          gen += u;
        }
      }
      m_liveness_map.insert(std::make_pair(b.label(), binding_t(kill, gen)));
    }
  }

  virtual varset_domain_t analyze(const basic_block_label_t &bb_id,
                                  varset_domain_t in) {
    auto it = m_liveness_map.find(bb_id);
    assert(it != m_liveness_map.end());
    in -= it->second.first;
    in += it->second.second;
    return in;
  }

  virtual std::string name() { return "Liveness"; }
};

/** Live variable analysis **/
template <typename CFG>
class liveness_analysis : public crab::iterators::killgen_fixpoint_iterator<
                              CFG, liveness_analysis_operations<CFG>> {

  using liveness_analysis_operations_t = liveness_analysis_operations<CFG>;
  using killgen_fixpoint_iterator_t =
      crab::iterators::killgen_fixpoint_iterator<
          CFG, liveness_analysis_operations_t>;

  liveness_analysis(const liveness_analysis<CFG> &other) = delete;
  liveness_analysis<CFG> &
  operator=(const liveness_analysis<CFG> &other) = delete;

  using basic_block_t = typename CFG::basic_block_t;
  
public:
  using basic_block_label_t = typename CFG::basic_block_label_t;
  using statement_t = typename CFG::statement_t;
  using varname_t = typename CFG::varname_t;
  typedef
      typename liveness_analysis_operations_t::varset_domain_t varset_domain_t;

private:
  // output of the analysis: map basic blocks to set of live
  // variables at the end of the blocks
  std::unordered_map<basic_block_label_t, varset_domain_t> _live_map;

public:
  liveness_analysis(CFG cfg) : killgen_fixpoint_iterator_t(cfg) {}

  void exec() {
    this->run();

    crab::ScopedCrabStats __st__("Liveness.store_results");
    _live_map.insert(make_move_iterator(this->out_begin()),
                     make_move_iterator(this->out_end()));

    CRAB_LOG(
        "liveness-live",
        for (auto p
             : boost::make_iterator_range(this->out_begin(), this->out_end())) {
          crab::outs() << basic_block_traits<basic_block_t>::to_string(p.first)
                       << " live variables=" << p.second << "\n";
          ;
        });

    this->release_memory();
  }

  varset_domain_t get(const basic_block_label_t &bb) const {
    auto it = _live_map.find(bb);
    if (it != _live_map.end()) {
      return it->second;
    } else {
      return varset_domain_t::bottom();
    }
  }

  void write(crab_os &o) const {
    o << "TODO: print liveness analysis results\n";
  }
};

template <typename CFG>
inline crab_os &operator<<(crab_os &o, const liveness_analysis<CFG> &l) {
  l.write(o);
  return o;
}

/**
 * Live and Dead variable analysis.
 **/
template <typename CFG> class live_and_dead_analysis {
public:
  using basic_block_label_t = typename CFG::basic_block_label_t;
  using basic_block_t = typename CFG::basic_block_t;
  using statement_t = typename CFG::statement_t;
  using varname_t = typename CFG::varname_t;
  using variable_t = typename CFG::variable_t;
  using varset_domain_t = varset_domain<variable_t>;

private:
  using liveness_analysis_t = liveness_analysis<CFG>;

  // the cfg
  CFG m_cfg;
  // liveness analysis
  std::unique_ptr<liveness_analysis_t> m_live;
  // precompute dead variables might be expensive so user can choose.
  bool m_ignore_dead;
  // map basic blocks to set of dead variables at the end of the
  // blocks
  std::unordered_map<basic_block_label_t, varset_domain_t> m_dead_map;

  // statistics
  unsigned m_max_live;
  unsigned m_total_live;
  unsigned m_total_blocks;

public:
  // for backward compatibility
  // XXX: maybe unused already
  using set_t = varset_domain_t;

  // If ignore_dead is true then dead symbols are not computed.
  live_and_dead_analysis(CFG cfg, bool ignore_dead = false)
      : m_cfg(cfg), m_live(new liveness_analysis_t(m_cfg)),
        m_ignore_dead(ignore_dead), m_max_live(0), m_total_live(0),
        m_total_blocks(0) {}

  live_and_dead_analysis(const live_and_dead_analysis &other) = delete;

  live_and_dead_analysis &
  operator=(const live_and_dead_analysis &other) = delete;

  void exec() {
    m_live->exec();
    if (!m_ignore_dead) {
      crab::ScopedCrabStats __st__("Liveness.precompute_dead_variables");
      /** Remove dead variables locally **/
      for (auto &bb : boost::make_iterator_range(m_cfg.begin(), m_cfg.end())) {
        varset_domain_t live_set = m_live->get(bb.label());
        if (live_set.is_bottom())
          continue;

        varset_domain_t dead_set = m_cfg.get_node(bb.label()).live();
        // dead variables = (USE(bb) U DEF(bb)) \ live_out(bb)
        dead_set -= live_set;
        CRAB_LOG("liveness", crab::outs()
		 << basic_block_traits<basic_block_t>::to_string(bb.label())
		 << " dead variables=" << dead_set << "\n";);
        m_dead_map.insert(std::make_pair(bb.label(), std::move(dead_set)));
        // update statistics
        m_total_live += live_set.size();
        m_max_live = std::max(m_max_live, live_set.size());
        m_total_blocks++;
      }
    }
  }

  // Return the set of live variables at the exit of block bb
  varset_domain_t get(const basic_block_label_t &bb) const {
    return m_live->get(bb);
  }

  // Return the set of dead variables at the exit of block bb
  varset_domain_t dead_exit(const basic_block_label_t &bb) const {
    if (m_ignore_dead) {
      CRAB_WARN("Dead variables were not precomputed during liveness analysis");
    }
    auto it = m_dead_map.find(bb);
    if (it == m_dead_map.end()) {
      return varset_domain_t();
    } else {
      return it->second;
    }
  }

  void get_stats(unsigned &total_live, unsigned &max_live_per_blk,
                 unsigned &avg_live_per_blk) const {
    total_live = m_total_live;
    max_live_per_blk = m_max_live;
    avg_live_per_blk =
        (m_total_blocks == 0 ? 0 : (int)m_total_live / m_total_blocks);
  }

  void write(crab_os &o) const {
    o << "TODO: printing dead variable analysis results\n";
  }
};

template <typename CFG>
inline crab_os &operator<<(crab_os &o, const live_and_dead_analysis<CFG> &l) {
  l.write(o);
  return o;
}

} // end namespace analyzer
} // end namespace crab
