#pragma once

#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>
#include <cassert>

class ExecutionPlanner {

public:
    static std::optional<ExecutionPlanner> instance; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

    static void initInst(unsigned numThreads, bool enableHT) {
        if(instance.has_value()) {
            throw std::runtime_error("Instance already initialized");
        }
        instance.emplace(numThreads, enableHT);
    }

    static const ExecutionPlanner& getInst() {
        return instance.value(); // NOLINT(bugprone-unchecked-optional-access)
    }

private:
    // private variables
    std::vector<unsigned> m_numaList;
    std::vector<std::vector<unsigned>> m_cpuPerNuma;
    unsigned m_cpuCnt;
    bool m_isNuma;


    // private member functions

    static std::vector<unsigned> getCPUList();

    struct HTInfo {
        struct CpuIDS {
            unsigned physicalPackageID;
            unsigned coreID;
            bool operator== (const CpuIDS &other) const {
                return coreID == other.coreID && physicalPackageID == other.physicalPackageID;
            }
            bool operator< (const CpuIDS &other) const {
                if(physicalPackageID == other.physicalPackageID) {
                    return coreID < other.coreID;
                }
                return physicalPackageID < other.physicalPackageID;
            }
            bool operator!= (const CpuIDS &other) const {
                return !((*this) == other);
            }
        };

        std::vector<CpuIDS> m_cpiIDS;
        //struct CoreIDToCpuMap {
        //    unsigned CoreID;
        //    unsigned cpu;
        //    bool operator< (const struct CoreIDToCpuMap &other) {
        //        if(CoreID == other.CoreID)
        //            return cpu < other.cpu;
        //        return CoreID < other.CoreID;
        //    }
        //};
        //std::vector<CoreIDToCpuMap> m_coreIDToCpu;

        static constexpr CpuIDS NOCPU = {std::numeric_limits<unsigned>::max(), 
                                         std::numeric_limits<unsigned>::max()};

        static HTInfo getInfo(const std::vector<unsigned> &cpuList);
    };

    // called inside constructor
    void solveNoNumaHt(unsigned numThreads);

    // called inside constructor
    void solveNoNumaNoHt(unsigned numThreads);

    // called inside constructor
    void solveNumaHt(unsigned numThreads);

    // called inside constructor
    void solveNumaNoHt(unsigned numThreads);

public:

    // static constexpr unsigned NONUMA = std::numeric_limits<unsigned>::max();

    ExecutionPlanner(unsigned numThreads, bool enableHT);

    [[nodiscard]] bool isNuma() const { return m_isNuma; }
    [[nodiscard]] auto getNumaList() const
    -> const std::vector<unsigned>& { return m_numaList; }
    // for NonNUMA system use numaNode = 0
    [[nodiscard]] auto getCpuListPerNuma(unsigned numaNode) const
    -> const std::vector<unsigned>& {
        assert(isNuma() || numaNode == 0);
        if(numaNode > m_cpuPerNuma.size()) {
            throw std::invalid_argument("numaNode is larger than numaNodes on system");
        }

        return m_cpuPerNuma[numaNode];
    }
    [[nodiscard]] unsigned getCpuCnt() const { return m_cpuCnt; }

    void printStats(std::ostream &out) const;

};
