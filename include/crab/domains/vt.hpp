#pragma once

#include <crab/domains/interval.hpp>
#include <crab/domains/sign.hpp>
#include <crab/numbers/bignums.hpp>

#include <map> 
#include <tuple>
#include <vector>

namespace crab {
    namespace domains {

        enum class variable_tendency {
            EQ, //Equal
            GT, //greater than
            LT, //less than
            DK,  //dont know

        };


        template <typename Number> class vt {
            static_assert(std::is_same<Number, ikos::z_number>::value,
                "Class vt only defined over ikos::z_number");

            using sign_t = sign<Number>;
            using vt_t = vt<Number>;
            using interval_t = ikos::interval<Number>;
            using bound_t = ikos::bound<Number>;

            sign_t m_sign;
            std::map<ikos::index_t, variable_tendency> m_tendency;                       

            constexpr Number get_zero() const { return Number(0); }
            constexpr Number get_plus_one() const { return Number(1); }
            constexpr Number get_minus_one() const { return Number(-1); }

        public:
            static sign_t sign_constructor(sign_interval si);

            explicit vt(sign_interval si);
            explicit vt(bool is_bottom);
            explicit vt(Number c);

            explicit vt(sign_interval si, std::vector<std::tuple<ikos::index_t, variable_tendency>> ttv);
            explicit vt(bool is_bottom, std::vector<std::tuple<ikos::index_t, variable_tendency>> ttv);
            explicit vt(Number c, std::vector<std::tuple<ikos::index_t, variable_tendency>> ttv);

            explicit vt(sign_interval si, std::vector<ikos::index_t> iv);
            explicit vt(bool is_bottom, std::vector<ikos::index_t> iv);
            explicit vt(Number c, std::vector<ikos::index_t> iv);

            explicit vt(sign_interval si, std::map<ikos::index_t, variable_tendency> tm);                                   
            explicit vt(bool is_bottom, std::map<ikos::index_t, variable_tendency> tm);                                 
            explicit vt(Number c, std::map<ikos::index_t, variable_tendency> tm);

            static vt_t bottom();
            static vt_t top();
            static vt_t bottom(std::vector<std::tuple<ikos::index_t, variable_tendency>> ttv);
            static vt_t top(std::vector<std::tuple<ikos::index_t, variable_tendency>> ttv);
            static vt_t bottom(std::vector<ikos::index_t> iv);
            static vt_t top(std::vector<ikos::index_t> iv);
            static vt_t bottom(std::map<ikos::index_t, variable_tendency> tm);
            static vt_t top(std::map<ikos::index_t, variable_tendency> tm);

            static vt_t tendency_bottom(const sign_t& s, std::map<ikos::index_t, variable_tendency> tm);
            static vt_t tendency_top(const sign_t& s, std::map<ikos::index_t, variable_tendency> tm);

            bool is_bottom() const;
            bool is_top() const;     

            std::map<ikos::index_t, variable_tendency> get_m_tendency() const;
            sign_t get_m_sign() const;
            static sign_interval get_sign_interval(const sign_t& s);
         
            vt_t from_interval(const interval_t& i) const;
            interval_t to_interval() const;

            // lattice operations
            bool operator<=(const vt_t& o) const;
            bool operator==(const vt_t& o) const;
            variable_tendency lub_vte(const variable_tendency r_1, const variable_tendency r_2) const;
            variable_tendency glb_vte(const variable_tendency r_1, const variable_tendency r_2) const;
            vt_t operator|(const vt_t& o) const;
            vt_t operator&(const vt_t& o) const;

            //sign negation
            static sign_t negate(const sign_t& s);

            //check if intersection between two set of signs is empty
            static bool empty_intersection(const sign_t& s_1, const sign_t& s_2);

            //addition r + s
            variable_tendency plus(variable_tendency r, const sign_t& s) const;
            //multiplication (r, s1) * s2
            variable_tendency times(variable_tendency r, const sign_t& s_1, const sign_t& s_2) const;
            //signed division (r, s1) / s2
            variable_tendency div(variable_tendency r, const sign_t& s_1, const sign_t& s_2) const;

            static int sign_set_size(const sign_t& s);
            static std::vector<sign_t> to_singleton_signs(const sign_t& s);

            // addition
            vt_t operator+(const vt_t& o) const;
            // subtraction
            vt_t operator-(const vt_t& o) const;
            // multiplication
            vt_t operator*(const vt_t& o) const;
            // signed division
            vt_t operator/(const vt_t& o) const;
            // division and remainder operations
            vt_t UDiv(const vt_t& o) const;
            vt_t SRem(const vt_t& o) const;
            vt_t URem(const vt_t& o) const;
            // bitwise operations
            vt_t And(const vt_t& o) const;
            vt_t Or(const vt_t& o) const;
            vt_t Xor(const vt_t& o) const;
            //shift operations
            vt_t Shl(const vt_t& o) const;
            vt_t LShr(const vt_t& o) const;
            vt_t AShr(const vt_t& o) const;

            void write(crab::crab_os& o) const;

            friend inline crab_os& operator<<(crab_os& o, const vt_t& c) {
                c.write(o);
                return o;
            }
        };
    } // namespace domains
} // namespace crab
