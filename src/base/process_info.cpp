//
// Created by acai on 6/4/22.
//

#include "process_info.h"
#include <algorithm>
#include <unistd.h>
#include <stdlib.h>
#include <sys/resource.h>
#include "file_utils.h"
#include "current_thead.h"
#include <pwd.h>
#include <dirent.h>
#include <sys/times.h>


namespace ac_muduo {
    namespace detail {
        timestamp g_start_time = timestamp::now();

        __thread int t_num_opened_files = 0;

        int fd_dir_filter(const struct dirent *d) {
            if (::isdigit(d->d_name[0])) {
                ++t_num_opened_files;
            }
            return 0;
        }

        __thread std::vector<pid_t> *t_pids = nullptr;

        int task_dir_filter(const struct dirent *d) {
            if (::isdigit(d->d_name[0])) {
                t_pids->push_back(atoi(d->d_name));
            }
            return 0;
        }

        int scan_dir(const char *dir_path, int(*filter)(const struct dirent *)) {
            struct dirent **namelist = nullptr;
            int result = ::scandir(dir_path, &namelist, filter, alphasort);
            assert(namelist == nullptr);
            return result;
        }

        int g_clock_ticks = static_cast<int>(::sysconf(_SC_CLK_TCK));
        int g_page_size = static_cast<int>(::sysconf(_SC_PAGE_SIZE));
    }
    namespace process_info {

        pid_t pid() {
            return ::getpid();
        }

        string pid_string() {
            char buf[32];
            snprintf(buf, sizeof(buf), "%d", pid());
            return buf;
        }

        uid_t uid() {
            return ::getuid();
        }

        string user_name() {
            struct passwd pwd;
            struct passwd *result = nullptr;
            char buf[8192];
            const char *name = "unknownuser";
            getpwuid_r(uid(), &pwd, buf, sizeof(buf), &result);

            if (result) {
                name = pwd.pw_name;
            }
            return name;
        }

        uid_t euid() {
            return ::geteuid();
        }

        timestamp start_time() {
            return detail::g_start_time;
        }

        int clock_ticks_per_seconds() {
            return detail::g_clock_ticks;
        }

        int page_size() {
            return detail::g_page_size;

        }

        bool is_debug_build() {
#ifdef NDEBUG
            return false;
#else
            return true;
#endif
        }

        std::string host_name() {

            char buf[256];
            if (::gethostname(buf, sizeof(buf)) == 0) {
                buf[sizeof(buf) - 1] = '\n';
                return buf;
            } else {
                return "unknownhost";
            }
        }


        std::string proc_name() {
            return proc_name(proc_stat()).as_string();
        }

        string_piece proc_name(const std::string &stat) {
            string_piece name;
            size_t lp = stat.find("(");
            size_t rp = stat.find(")");

            if (lp != string::npos && rp != string::npos && lp < rp) {
                name.set(stat.data() + lp + 1, static_cast<int>(rp - lp - 1));
            }
            return name;
        }

        std::string proc_status() {
            string result;
            file_utils::read_file("/proc/self/status", 65535, &result);
            return result;
        }

        std::string proc_stat() {
            string result;
            file_utils::read_file("/proc/self/stat", 65535, &result);
            return result;
        }

        std::string exe_path() {
            string result;
            char buf[1024];
            ssize_t n = ::readlink("/proc/self/exe", buf, sizeof buf);
            if (n > 0) {
                result.assign(buf, n);
            }
            return result;
        }


        int opened_files() {
            detail::t_num_opened_files = 0;
            detail::scan_dir("/proc/self/fd", detail::fd_dir_filter);
            return detail::t_num_opened_files;
        }

        int max_open_files() {
            struct rlimit rl;
            if (::getrlimit(RLIMIT_NOFILE, &rl)) {
                return opened_files();
            } else {
                return static_cast<int>(rl.rlim_cur);
            }
        }

        cpu_time_t cpu_time() {
            cpu_time_t t;
            struct tms tms;
            if (::times(&tms) >= 0) {

                const double hz = static_cast<double>(clock_ticks_per_seconds());
                t.user_seconds = static_cast<double>(tms.tms_utime) / hz;
                t.system_seconds = static_cast<double>(tms.tms_stime) / hz;
            }

            return t;
        }

        int num_thread() {

            int result = 0;
            string status = proc_status();
            size_t pos = status.find("Threads:");
            if (pos != string::npos) {
                result = ::atoi(status.data() + pos + 8);
            }
            return result;
        }

        std::vector<pid_t> threads() {
            std::vector<pid_t> result;
            detail::t_pids = &result;
            detail::scan_dir("/proc/self/task", detail::task_dir_filter);
            detail::t_pids = nullptr;

            std::sort(result.begin(), result.end());
            return result;
        }

        string thread_stat() {
            char buf[64];
            snprintf(buf, sizeof buf, "/proc/self/task/%d/stat", current_thread::tid());
            string result;
            file_utils::read_file(buf, 65535, &result);
            return result;
        }

    }
}
