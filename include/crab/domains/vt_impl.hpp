#pragma once

#include <crab/domains/vt.hpp>
#include <crab/support/debug.hpp>
#include <crab/support/stats.hpp>

#include <boost/optional.hpp>
#include <map>
#include <iostream>

//??? TODO variable_tendency enum ausgliedern als eigene Klasse mit eigenen Funktionen ??? baut allerdings auf sign auf
namespace crab {
    namespace domains {    

        template <typename Number> sign<Number> vt<Number>::sign_constructor(sign_interval si) {
            switch (si) {
            case sign_interval::LTZ: return sign<Number>::mk_less_than_zero();
            case sign_interval::EQZ: return sign<Number>::mk_equal_zero();
            case sign_interval::GTZ: return sign<Number>::mk_greater_than_zero();
            case sign_interval::NEZ: return sign<Number>::mk_not_equal_zero();
            case sign_interval::GEZ: return sign<Number>::mk_greater_or_equal_than_zero();
            case sign_interval::LEZ: return sign<Number>::mk_less_or_equal_than_zero();
            case sign_interval::BOT: return sign<Number>::bottom();
            case sign_interval::TOP: return sign<Number>::top();
            default: CRAB_ERROR("vt::sign_constructor unreachable 1");
            }
            CRAB_ERROR("vt::sign_constructor unreachable 2");
        }

        template <typename Number> vt<Number>::vt(sign_interval si) : m_sign(sign_constructor(si)) {}

        template <typename Number>
        vt<Number>::vt(bool is_bottom) : m_sign(sign<Number>(is_bottom)) {}

        template <typename Number>
        vt<Number>::vt(Number c) : m_sign(sign<Number>(c)) {}

        template <typename Number>
        vt<Number>::vt(sign_interval si, std::vector<std::tuple<ikos::index_t, variable_tendency>> ttv) : m_sign(sign_constructor(si)) {
            for (auto tt : ttv) {
                m_tendency[std::get<0>(tt)] = std::get<1>(tt);
            }
        }

        template <typename Number>
        vt<Number>::vt(bool is_bottom, std::vector<std::tuple<ikos::index_t, variable_tendency>> ttv) : m_sign(sign<Number>(is_bottom)) {
            for (auto tt : ttv) {
                if (is_bottom) {
                    m_tendency[std::get<0>(tt)] = variable_tendency::EQ;
                }
                else {
                    m_tendency[std::get<0>(tt)] = variable_tendency::DK;
                }                
            }
        }

        template <typename Number>
        vt<Number>::vt(Number c, std::vector<std::tuple<ikos::index_t, variable_tendency>> ttv) : m_sign(sign<Number>(c)) {
            for (auto tt : ttv) {
                m_tendency[std::get<0>(tt)] = variable_tendency::DK;
            }
        }

        template <typename Number>
        vt<Number>::vt(sign_interval si, std::vector<ikos::index_t> iv) : m_sign(sign_constructor(si)) {
            for (auto i : iv) {
                m_tendency[i] = variable_tendency::EQ;
            }
        }

        template <typename Number>
        vt<Number>::vt(bool is_bottom, std::vector<ikos::index_t> iv) : m_sign(sign<Number>(is_bottom)) {
            for (auto i : iv) {
                if (is_bottom) {
                    m_tendency[i] = variable_tendency::EQ;
                }
                else {
                    m_tendency[i] = variable_tendency::DK;
                }               
            }
        }

        template <typename Number>
        vt<Number>::vt(Number c, std::vector<ikos::index_t> iv) : m_sign(sign<Number>(c)) {
            for (auto i : iv) {
                m_tendency[i] = variable_tendency::DK;
            }
        }

        template <typename Number>
        vt<Number>::vt(sign_interval si, std::map<ikos::index_t, variable_tendency> tm) : m_sign(sign_constructor(si)) {
            for (auto& t : tm) {
                m_tendency[t.first] = t.second;
            }
        }

        template <typename Number>
        vt<Number>::vt(bool is_bottom, std::map<ikos::index_t, variable_tendency> tm) : m_sign(sign<Number>(is_bottom)) {
            for (auto& t : tm) {
                if (is_bottom) {
                    m_tendency[t.first] = variable_tendency::EQ;
                }
                else {
                    m_tendency[t.first] = variable_tendency::DK;
                }              
            }
        }

        template <typename Number>
        vt<Number>::vt(Number c, std::map<ikos::index_t, variable_tendency> tm) : m_sign(sign<Number>(c)) {
            for (auto& t : tm) {
                m_tendency[t.first] = variable_tendency::DK;
            }
        }
        
        template <typename Number> vt<Number> vt<Number>::bottom() {
            return vt<Number>(true);
        }

        
        template <typename Number> vt<Number> vt<Number>::top() {
            return vt<Number>(false);
        }

        template <typename Number> vt<Number> vt<Number>::bottom(std::vector<std::tuple<ikos::index_t, variable_tendency>> ttv) {
            return vt<Number>(true, ttv);
        }

        
        template <typename Number> vt<Number> vt<Number>::top(std::vector<std::tuple<ikos::index_t, variable_tendency>> ttv) {
            return vt<Number>(false, ttv);
        }

        template <typename Number> vt<Number> vt<Number>::bottom(std::vector<ikos::index_t> iv) {
            return vt<Number>(true, iv);
        }

        
        template <typename Number> vt<Number> vt<Number>::top(std::vector<ikos::index_t> iv) {
            return vt<Number>(false, iv);
        }

        template <typename Number> vt<Number> vt<Number>::bottom(std::map<ikos::index_t, variable_tendency> tm) {
            return vt<Number>(true, tm);
        }


        template <typename Number> vt<Number> vt<Number>::top(std::map<ikos::index_t, variable_tendency> tm) {
            return vt<Number>(false, tm);
        }

        template <typename Number> vt<Number> vt<Number>::tendency_bottom(const sign<Number> &s, std::map<ikos::index_t, variable_tendency> tm) {
            return vt<Number>(get_sign_interval(s), bottom(tm).m_tendency);
        }


        template <typename Number> vt<Number> vt<Number>::tendency_top(const sign<Number>& s, std::map<ikos::index_t, variable_tendency> tm) {
            return vt<Number>(get_sign_interval(s), top(tm).m_tendency);
        }

        //TODO
        //???  define bottom as sign_is_bottom for crab ???
        template <typename Number> bool vt<Number>::is_bottom() const {
            bool result = true;
            if (!m_sign.is_bottom()) {
                result = false;
            }
            for (auto &t : m_tendency) {
                if (t.second != variable_tendency::EQ) {
                    result = false;
                }
            }

            return result;
        }

        template <typename Number> bool vt<Number>::is_top() const {
            bool result = true;
            if (!m_sign.is_top()) {
                result = false;
            }
            for (const auto& t : m_tendency) {
                if (t.second != variable_tendency::DK) {
                    result = false;
                }
            }

            return result;
        }       

        template <typename Number> std::map<ikos::index_t, variable_tendency> vt<Number>::get_m_tendency() const {
            return m_tendency;
        }   

        template <typename Number> sign<Number> vt<Number>::get_m_sign() const {
            return sign_constructor(get_sign_interval(m_sign));
        }

        template <typename Number> sign_interval vt<Number>::get_sign_interval(const sign<Number> &s) { 
            if (s.is_bottom()) {
                return sign_interval::BOT;
            }
            else if (s.is_top()) {
                return sign_interval::TOP;
            }
            else if (s.equal_zero()) {
                return sign_interval::EQZ;
            }
            else if (s.less_than_zero()) {
                return sign_interval::LTZ;
            }
            else if (s.greater_than_zero()) {
                return sign_interval::GTZ;
            }
            else if (s.less_or_equal_than_zero()) {
                return sign_interval::LEZ;
            }
            else if (s.greater_or_equal_than_zero()) {
                return sign_interval::GEZ;
            }
            else if (s.not_equal_zero()) {
                return sign_interval::NEZ;
            }
            CRAB_ERROR("vt::get_sign_interval unreachable");
            
        }

        /* MODIFY HERE if not integers */
        //TODO
        //m_tendency stays empty
        template <typename Number>
        vt<Number>
            vt<Number>::from_interval(const ikos::interval<Number>& i) const {
            Number zero(get_zero());
            Number plus_one(get_plus_one());
            Number minus_one(get_minus_one());
            if (i.is_bottom()) {
                return vt<Number>::bottom();
            }
            else if (i.is_top()) {
                return vt<Number>::top();
            }
            else if (i <= ikos::interval<Number>(zero, zero)) {
                return vt<Number>(sign_interval::EQZ);
            }
            else if (i <=
                ikos::interval<Number>(bound_t::minus_infinity(), minus_one)) {
                return vt<Number>(sign_interval::LTZ);
            }
            else if (i <= ikos::interval<Number>(bound_t::minus_infinity(), zero)) {
                return vt<Number>(sign_interval::LEZ);
            }
            else if (i <= ikos::interval<Number>(plus_one, bound_t::plus_infinity())) {
                return vt<Number>(sign_interval::GTZ);
            }
            else if (i <= ikos::interval<Number>(zero, bound_t::plus_infinity())) {
                return vt<Number>(sign_interval::GEZ);
            }
            else {
                // unreachable
                return vt<Number>::top();
            }
        }

        /* MODIFY HERE if not integers */
        //TODO
        template <typename Number>
        ikos::interval<Number> vt<Number>::to_interval() const {
            Number zero(get_zero());
            Number plus_one(get_plus_one());
            Number minus_one(get_minus_one());
            if (m_sign.is_bottom()) {
                return ikos::interval<Number>::bottom();
            }
            else if (m_sign.is_top()) {
                return ikos::interval<Number>::top();
            }
            else if (m_sign.equal_zero()) {
                return ikos::interval<Number>(zero);
            }
            else if (m_sign.less_than_zero()) {
                return ikos::interval<Number>(bound_t::minus_infinity(), minus_one);
            }
            else if (m_sign.greater_than_zero()) {
                return ikos::interval<Number>(plus_one, bound_t::plus_infinity());
            }
            else if (m_sign.less_or_equal_than_zero()) {
                return ikos::interval<Number>(bound_t::minus_infinity(), zero);
            }
            else if (m_sign.greater_or_equal_than_zero()) {
                return ikos::interval<Number>(zero, bound_t::plus_infinity());
            }
            else {
                // we cannot express not_equal_zero in an interval
                return ikos::interval<Number>::top();
            }
                      
        }

        template <typename Number>
        bool vt<Number>::operator<=(const vt<Number>& o) const {
            if (is_bottom() || o.is_top()) {
                return true;
            }
            else if (o.is_bottom() || is_top()) {
                return false;
            }
            else {
                // operands are not either top or bottom
                if (m_tendency.size() != o.m_tendency.size()) {
                    //incomparable since size of mappings differ
                    return false;
                }
                if (!(m_sign <= o.m_sign)) {
                    return false;
                }
                for (const auto& t : m_tendency) {
                    if (t.second == variable_tendency::GT) {
                        if (o.m_tendency.at(t.first) == variable_tendency::EQ || o.m_tendency.at(t.first) == variable_tendency::LT) {
                            return false;
                        }
                    }
                    if (t.second == variable_tendency::LT) {
                        if (o.m_tendency.at(t.first) == variable_tendency::EQ || o.m_tendency.at(t.first) == variable_tendency::GT) {
                            return false;
                        }
                    }
                    if (t.second == variable_tendency::DK) {
                        if (o.m_tendency.at(t.first) != variable_tendency::DK) {
                            return false;
                        }
                    }
                }
                return true;               
            }
        }

        template <typename Number>
        bool vt<Number>::operator==(const vt<Number>& o) const {
            bool result = true;
            if (!(m_sign == o.m_sign)) {
                result = false;
            }
            if (m_tendency.size() != o.m_tendency.size()) {
                //incomparable since size of mappings differ
                return false;
            }
            for (const auto& t : m_tendency) {
                if (!(t.second == o.m_tendency.at(t.first))) {
                    result = false;
                }
            }

            return result;
        }

        //LUB for variable_tendency enum
        template <typename Number>
        variable_tendency vt<Number>::lub_vte(const variable_tendency r_1, const variable_tendency r_2) const {
            if (r_1 == variable_tendency::EQ) {
                return r_2;
            }
            else if (r_1 == variable_tendency::GT) {
                if (r_2 == variable_tendency::EQ || r_2 == variable_tendency::GT) {
                    return r_1;
                }
                else {
                    return variable_tendency::DK;
                }
            }
            else if (r_1 == variable_tendency::LT) {
                if (r_2 == variable_tendency::EQ || r_2 == variable_tendency::LT) {
                    return r_1;
                }
                else {
                    return variable_tendency::DK;
                }
            }
            else /* (r_1 == variable_tendency::DK)*/ {
                return r_1;
            }
        }

        //GLB for variable_tendency enum
        template <typename Number>
        variable_tendency vt<Number>::glb_vte(const variable_tendency r_1, const variable_tendency r_2) const {
            if (r_1 == variable_tendency::EQ) {
                return r_1;
            }
            else if (r_1 == variable_tendency::GT) {
                if (r_2 == variable_tendency::DK || r_2 == variable_tendency::GT) {
                    return r_1;
                }
                else {
                    return variable_tendency::EQ;
                }
            }
            else if (r_1 == variable_tendency::LT) {
                if (r_2 == variable_tendency::DK || r_2 == variable_tendency::LT) {
                    return r_1;
                }
                else {
                    return variable_tendency::EQ;
                }
            }
            else /* (r_1 == variable_tendency::DK)*/ {
                return r_2;
            }
        }

        //LUB for vt
        template <typename Number>
        vt<Number> vt<Number>::operator|(const vt<Number>& o) const {                       
            if (is_bottom() || o.is_top()) {
                return o;
            }
            else if (is_top() || o.is_bottom()) {
                return *this;
            }
            else {
                
                // operands are not either top or bottom              
                if (m_tendency.size() != o.m_tendency.size()) {                   
                    CRAB_WARN("vt::operator| map sizes differ");
                    sign_interval sv_warn = get_sign_interval(m_sign.operator|(o.m_sign));
                    if (m_tendency.size() > 0) {
                        return vt<Number>(sv_warn, m_tendency);
                    }
                    else {
                        return vt<Number>(sv_warn, o.m_tendency);
                    }
                }

                std::vector<std::tuple<ikos::index_t, variable_tendency>> ttv;

                for (const auto& t : m_tendency) {    
                    variable_tendency lub = lub_vte(t.second, o.m_tendency.at(t.first));
                    ttv.push_back(std::make_tuple(t.first, lub));                    
                }
                
                sign_interval sv = get_sign_interval(m_sign.operator|(o.m_sign));

                return vt<Number>(sv, ttv);                                                                                             
            }
            CRAB_ERROR("vt::operator| unreachable");
        }
       
        //GLB for vt
        template <typename Number>
        vt<Number> vt<Number>::operator&(const vt<Number>& o) const {
            if (is_bottom() || o.is_top())
                return *this;
            else if (is_top() || o.is_bottom()) {
                return o;
            }
            else {
                // operands are not either top or bottom
                if (m_tendency.size() != o.m_tendency.size()) {
                    CRAB_ERROR("vt::operator& map sizes differ");
                }

                std::vector<std::tuple<ikos::index_t, variable_tendency>> ttv;

                for (const auto& t : m_tendency) {
                    variable_tendency glb = glb_vte(t.second, o.m_tendency.at(t.first));
                    ttv.push_back(std::make_tuple(t.first, glb));
                }

                sign_interval sv = get_sign_interval(m_sign.operator&(o.m_sign));

                return vt<Number>(sv, ttv);              
            }
            CRAB_ERROR("vt::operator& unreachable");
        }
        
        template <typename Number>
        sign<Number> vt<Number>::negate(const sign<Number>& s) {
            switch (get_sign_interval(s)) {
            case sign_interval::LTZ: return sign<Number>::mk_greater_than_zero();
            case sign_interval::EQZ: return sign<Number>::mk_equal_zero();
            case sign_interval::GTZ: return sign<Number>::mk_less_than_zero();
            case sign_interval::NEZ: return sign<Number>::mk_not_equal_zero();
            case sign_interval::GEZ: return sign<Number>::mk_less_or_equal_than_zero();
            case sign_interval::LEZ: return sign<Number>::mk_greater_or_equal_than_zero();
            case sign_interval::BOT: return sign<Number>::bottom();
            case sign_interval::TOP: return sign<Number>::top();
            default: CRAB_ERROR("vt::negate unreachable 1");
            }
            CRAB_ERROR("vt::negate unreachable 2");
        }

        template <typename Number>
        bool vt<Number>::empty_intersection(const sign<Number>& s_1, const sign<Number>& s_2) {            
            if (s_2.less_than_zero()) {
                switch (get_sign_interval(s_1)) {
                case sign_interval::EQZ:
                case sign_interval::GTZ:
                case sign_interval::GEZ:
                case sign_interval::BOT:
                    return true;
                case sign_interval::LTZ:
                case sign_interval::NEZ:                
                case sign_interval::LEZ:
                case sign_interval::TOP:
                    return false;
                default: CRAB_ERROR("vt::empty_intersection unreachable 1");
                }
            }
            else if (s_2.greater_than_zero()) {
                switch (get_sign_interval(s_1)) {
                case sign_interval::EQZ:
                case sign_interval::LTZ:
                case sign_interval::LEZ:
                case sign_interval::BOT:
                    return true;
                case sign_interval::GTZ:
                case sign_interval::NEZ:
                case sign_interval::GEZ:
                case sign_interval::TOP:
                    return false;
                default: CRAB_ERROR("vt::empty_intersection unreachable 2");
                }
            }
            else if (s_2.equal_zero()) {
                switch (get_sign_interval(s_1)) {
                case sign_interval::LTZ:
                case sign_interval::GTZ:
                case sign_interval::NEZ:
                case sign_interval::BOT:
                    return true;
                case sign_interval::EQZ:
                case sign_interval::LEZ:
                case sign_interval::GEZ:
                case sign_interval::TOP:
                    return false;
                default: CRAB_ERROR("vt::empty_intersection unreachable 3");
                }
            }
            CRAB_ERROR("vt::empty_intersection unreachable 4");
            //implement more cases if needed           
        }

        template <typename Number>
        variable_tendency vt<Number>::plus(variable_tendency r, const sign<Number>& s) const {            
            if (r == variable_tendency::EQ && s.equal_zero()) {
                return variable_tendency::EQ;
            }
            else if ((r == variable_tendency::GT && empty_intersection(s, sign<Number>::mk_less_than_zero())) || (r == variable_tendency::EQ && s.greater_than_zero())) {
                return variable_tendency::GT;
            }
            else if ((r == variable_tendency::LT && empty_intersection(s, sign<Number>::mk_greater_than_zero())) || (r == variable_tendency::EQ && s.less_than_zero())) {
                return variable_tendency::LT;
            }
            else /*otherwise*/ {
                return variable_tendency::DK;
            }
        }   

        template <typename Number>
        variable_tendency vt<Number>::times(variable_tendency r, const sign<Number>& s_1, const sign<Number>& s_2) const {
            if (r == variable_tendency::EQ && s_1.equal_zero()) {
                return variable_tendency::EQ;
            }
            else if ((r == variable_tendency::EQ && ((s_1.greater_than_zero() && s_2.greater_than_zero()) || (s_1.less_than_zero() && empty_intersection(s_2, sign<Number>::mk_greater_than_zero()))))
                || (r == variable_tendency::GT && (s_1.equal_zero() || s_1 == s_2 || (s_1.less_than_zero() && s_2.equal_zero())))) {
                return variable_tendency::GT;
            }
            else if ((r == variable_tendency::EQ && ((s_1.less_than_zero() && s_2.greater_than_zero()) || (s_1.greater_than_zero() && empty_intersection(s_2, sign<Number>::mk_greater_than_zero()))))
                || (r == variable_tendency::LT && (s_1.equal_zero() || s_1 == negate(s_2) || (s_1.greater_than_zero() && s_2.equal_zero())))) {
                return variable_tendency::LT;
            }
            else /*otherwise*/ {
                return variable_tendency::DK;
            }
        }

        template <typename Number>
        variable_tendency vt<Number>::div(variable_tendency r, const sign<Number>& s_1, const sign<Number>& s_2) const {             
            if (r == variable_tendency::EQ && s_1.equal_zero() && empty_intersection(s_2, sign<Number>::mk_equal_zero())) {
                return variable_tendency::EQ;
            }
            else if ((r == variable_tendency::EQ && s_1.less_than_zero() && empty_intersection(s_2, sign<Number>::mk_equal_zero()))
                || (r == variable_tendency::GT && empty_intersection(s_1, sign<Number>::mk_greater_than_zero()) && empty_intersection(s_2, sign<Number>::mk_equal_zero()))) {
                return variable_tendency::GT;
            }
            else if ((r == variable_tendency::EQ && s_1.greater_than_zero() && empty_intersection(s_2, sign<Number>::mk_equal_zero()))
                || (r == variable_tendency::LT && empty_intersection(s_1, sign<Number>::mk_less_than_zero()) && empty_intersection(s_2, sign<Number>::mk_equal_zero()))) {
                return variable_tendency::LT;
            }
            else /*otherwise*/ {
                return variable_tendency::DK;
            }
        }

        template <typename Number>
        int vt<Number>::sign_set_size(const sign<Number>& s) {
            switch (get_sign_interval(s)) {
            case sign_interval::BOT:
                return 0;
            case sign_interval::EQZ:
            case sign_interval::LTZ: 
            case sign_interval::GTZ:
                return 1;
            case sign_interval::NEZ:
            case sign_interval::LEZ:
            case sign_interval::GEZ:
                return 2;                      
            case sign_interval::TOP:
                return 3;
            default: CRAB_ERROR("vt::sign_set_size unreachable 1");                
            }
            CRAB_ERROR("vt::sign_set_size unreachable 2");
        }

        template <typename Number>
        std::vector<sign<Number>> vt<Number>::to_singleton_signs(const sign<Number>& s) {
            std::vector<sign<Number>> singleton_signs;
            switch (get_sign_interval(s)) {
            case sign_interval::BOT:
                singleton_signs.push_back(sign<Number>::bottom()); break;
            case sign_interval::EQZ:
                singleton_signs.push_back(sign<Number>::mk_equal_zero()); break;
            case sign_interval::LTZ:
                singleton_signs.push_back(sign<Number>::mk_less_than_zero()); break;
            case sign_interval::GTZ:
                singleton_signs.push_back(sign<Number>::mk_greater_than_zero()); break;
            case sign_interval::NEZ:
                singleton_signs.push_back(sign<Number>::mk_less_than_zero());
                singleton_signs.push_back(sign<Number>::mk_greater_than_zero()); break;
            case sign_interval::LEZ:
                singleton_signs.push_back(sign<Number>::mk_less_than_zero());
                singleton_signs.push_back(sign<Number>::mk_equal_zero()); break;
            case sign_interval::GEZ:
                singleton_signs.push_back(sign<Number>::mk_greater_than_zero());
                singleton_signs.push_back(sign<Number>::mk_equal_zero()); break;
            case sign_interval::TOP:
                singleton_signs.push_back(sign<Number>::mk_less_than_zero());
                singleton_signs.push_back(sign<Number>::mk_greater_than_zero());
                singleton_signs.push_back(sign<Number>::mk_equal_zero()); break;
            default: CRAB_ERROR("vt::sign_set_size unreachable 1");
            }
            return singleton_signs;
        }

        // addition
        template <typename Number>
        vt<Number> vt<Number>::operator+(const vt<Number>& o) const {
            if ((m_sign.is_bottom() || o.m_sign.is_bottom())) {
                if (m_tendency.size() > 0) {
                    return bottom(m_tendency);
                }
                else {
                    return bottom(o.m_tendency);
                }               
            }

            if (m_tendency.size() != o.m_tendency.size()) {
                CRAB_ERROR("vt::operator+ map sizes differ");
            }
           
            std::vector<sign<Number>> singleton_signs_s_1 = to_singleton_signs(m_sign);
            std::vector<sign<Number>> singleton_signs_s_2 = to_singleton_signs(o.m_sign);
            std::vector<std::tuple<ikos::index_t, variable_tendency>> ttv;

            for (const auto& t : m_tendency) {
                variable_tendency lub_1 = variable_tendency::EQ;
                variable_tendency lub_2 = variable_tendency::EQ;                
                for (auto s : singleton_signs_s_2) {
                    lub_1 = lub_vte(lub_1, plus(t.second, s));
                }
                for (auto s : singleton_signs_s_1) {
                    lub_2 = lub_vte(lub_2, plus(o.m_tendency.at(t.first), s));
                }

                variable_tendency glb = glb_vte(lub_1, lub_2);             

                ttv.push_back(std::make_tuple(t.first, glb));
            }

            sign_interval si = get_sign_interval(m_sign.operator+(o.m_sign));
            
            return vt<Number>(si, ttv);                                          
        }

        // subtraction
        template <typename Number>
        vt<Number> vt<Number>::operator-(const vt<Number>& o) const {
            if ((m_sign.is_bottom() || o.m_sign.is_bottom()))  {
                if (m_tendency.size() > 0) {
                    return bottom(m_tendency);
                }
                else {
                    return bottom(o.m_tendency);
                }
            }

            if (m_tendency.size() != o.m_tendency.size()) {
                CRAB_ERROR("vt::operator- map sizes differ");
            }
           
            std::vector<sign<Number>> singleton_signs_s_2 = to_singleton_signs(o.m_sign);
            std::vector<std::tuple<ikos::index_t, variable_tendency>> ttv;

            for (const auto& t : m_tendency) {
                variable_tendency lub = variable_tendency::EQ;
                for (auto s : singleton_signs_s_2) {
                    lub = lub_vte(lub, plus(t.second, negate(s)));
                }                              

                ttv.push_back(std::make_tuple(t.first, lub));
            }

            sign_interval si = get_sign_interval(m_sign.operator-(o.m_sign));

            return vt<Number>(si, ttv);
        }

        // multiplication
        template <typename Number>
        vt<Number> vt<Number>::operator*(const vt<Number>& o) const {
            if ((m_sign.is_bottom() || o.m_sign.is_bottom())) {
                if (m_tendency.size() > 0) {
                    return bottom(m_tendency);
                }
                else {
                    return bottom(o.m_tendency);
                }
            }

            if (m_tendency.size() != o.m_tendency.size()) {
                CRAB_ERROR("vt::operator* map sizes differ");
            }

            std::vector<sign<Number>> singleton_signs_s_1 = to_singleton_signs(m_sign);
            std::vector<sign<Number>> singleton_signs_s_2 = to_singleton_signs(o.m_sign);
            std::vector<std::tuple<ikos::index_t, variable_tendency>> ttv;

            for (const auto& t : m_tendency) {
                variable_tendency lub_1 = variable_tendency::EQ;
                variable_tendency lub_2 = variable_tendency::EQ;
                for (auto s_1 : singleton_signs_s_1) {
                    for (auto s_2 : singleton_signs_s_2) {
                        lub_1 = lub_vte(lub_1, times(t.second, s_1, s_2));
                    }                   
                }
                for (auto s_1 : singleton_signs_s_1) {
                    for (auto s_2 : singleton_signs_s_2) {
                        lub_2 = lub_vte(lub_2, times(o.m_tendency.at(t.first), s_2, s_1));
                    }
                }

                variable_tendency glb = glb_vte(lub_1, lub_2);

                ttv.push_back(std::make_tuple(t.first, glb));
            }

            sign_interval si = get_sign_interval(m_sign.operator*(o.m_sign));

            return vt<Number>(si, ttv);
        }
      
        // signed division
        template <typename Number>
        vt<Number> vt<Number>::operator/(const vt<Number>& o) const {
            if ((m_sign.is_bottom() || o.m_sign.is_bottom()))  {
                if (m_tendency.size() > 0) {
                    return bottom(m_tendency);
                }
                else {
                    return bottom(o.m_tendency);
                }
            }

            if (m_tendency.size() != o.m_tendency.size()) {
                CRAB_ERROR("vt::operator/ map sizes differ");
            }

            std::vector<sign<Number>> singleton_signs_s_1 = to_singleton_signs(m_sign);
            std::vector<sign<Number>> singleton_signs_s_2 = to_singleton_signs(o.m_sign);
            std::vector<std::tuple<ikos::index_t, variable_tendency>> ttv;

            for (const auto& t : m_tendency) {
                variable_tendency lub = variable_tendency::EQ;                
                for (auto s_1 : singleton_signs_s_1) {
                    for (auto s_2 : singleton_signs_s_2) {
                        lub = lub_vte(lub, div(t.second, s_1, s_2));
                    }
                }                

                ttv.push_back(std::make_tuple(t.first, lub));
            }

            sign_interval si = get_sign_interval(m_sign.operator/(o.m_sign));

            return vt<Number>(si, ttv);
        }

        // division and remainder operations

        //TODO
        template <typename Number>
        vt<Number> vt<Number>::UDiv(const vt<Number>& o) const {
            if (m_sign.is_bottom() || o.m_sign.is_bottom()) {
                if (m_tendency.size() > 0) {
                    return bottom(m_tendency);
                }
                else {
                    return bottom(o.m_tendency);
                }
            }
            return tendency_top(m_sign.UDiv(o.m_sign), m_tendency);
        }

        //TODO
        template <typename Number>
        vt<Number> vt<Number>::SRem(const vt<Number>& o) const {
            if (m_sign.is_bottom() || o.m_sign.is_bottom()) {
                if (m_tendency.size() > 0) {
                    return bottom(m_tendency);
                }
                else {
                    return bottom(o.m_tendency);
                }
            }
            return tendency_top(m_sign.SRem(o.m_sign), m_tendency);
        }

        //TODO
        template <typename Number>
        vt<Number> vt<Number>::URem(const vt<Number>& o) const {
            if (m_sign.is_bottom() || o.m_sign.is_bottom()) {
                if (m_tendency.size() > 0) {
                    return bottom(m_tendency);
                }
                else {
                    return bottom(o.m_tendency);
                }
            }
            return tendency_top(m_sign.URem(o.m_sign), m_tendency);
        }

        //TODO
        // bitwise operations
        template <typename Number>
        vt<Number> vt<Number>::And(const vt<Number>& o) const {
            if (m_sign.is_bottom() || o.m_sign.is_bottom()) {
                if (m_tendency.size() > 0) {
                    return bottom(m_tendency);
                }
                else {
                    return bottom(o.m_tendency);
                }
            }
            return tendency_top(m_sign.And(o.m_sign), m_tendency);
        }

        //TODO
        template <typename Number>
        vt<Number> vt<Number>::Or(const vt<Number>& o) const {
            if (m_sign.is_bottom() || o.m_sign.is_bottom()) {
                if (m_tendency.size() > 0) {
                    return bottom(m_tendency);
                }
                else {
                    return bottom(o.m_tendency);
                }
            }
            return tendency_top(m_sign.Or(o.m_sign), m_tendency);
        }

        //TODO
        template <typename Number>
        vt<Number> vt<Number>::Xor(const vt<Number>& o) const {
            if (m_sign.is_bottom() || o.m_sign.is_bottom()) {
                if (m_tendency.size() > 0) {
                    return bottom(m_tendency);
                }
                else {
                    return bottom(o.m_tendency);
                }
            }
            return tendency_top(m_sign.Xor(o.m_sign), m_tendency);
        }

        //TODO
        //Shift Left
        template <typename Number>
        vt<Number> vt<Number>::Shl(const vt<Number>& o) const {
            if (m_sign.is_bottom() || o.m_sign.is_bottom()) {
                if (m_tendency.size() > 0) {
                    return bottom(m_tendency);
                }
                else {
                    return bottom(o.m_tendency);
                }
            }
            return tendency_top(m_sign.Shl(o.m_sign), m_tendency);
        }

        //TODO
        //logical shift right
        //filling the highest bits with zeros
        template <typename Number>
        vt<Number> vt<Number>::LShr(const vt<Number>& o) const {
            if (m_sign.is_bottom() || o.m_sign.is_bottom()) {
                if (m_tendency.size() > 0) {
                    return bottom(m_tendency);
                }
                else {
                    return bottom(o.m_tendency);
                }
            }
            return tendency_top(m_sign.LShr(o.m_sign), m_tendency);
        }

        //TODO
        //arithmetic shift right
        template <typename Number>
        vt<Number> vt<Number>::AShr(const vt<Number>& o) const {
            if (m_sign.is_bottom() || o.m_sign.is_bottom()) {
                if (m_tendency.size() > 0) {
                    return bottom(m_tendency);
                }
                else {
                    return bottom(o.m_tendency);
                }
            }
            return tendency_top(m_sign.AShr(o.m_sign), m_tendency);
        }

        
        template <typename Number> void vt<Number>::write(crab::crab_os& o) const {
            o << "([";
            int counter = 1;
            for (const auto& t : m_tendency) {
                o << t.first << " -> ";
                switch (t.second) {
                case variable_tendency::EQ:
                    o << "r="; break;
                case variable_tendency::GT:
                    o << "r>="; break;
                case variable_tendency::LT:
                    o << "r<="; break;
                case variable_tendency::DK:
                    o << "r?"; break;
                }
                if (counter < m_tendency.size()) {
                    o << " | ";
                }
                counter++;
            }        
            o << "], ";
            m_sign.write(o);
            o << ")";
        }

    } // namespace domains
} // namespace crab
