//
// Created by acai on 6/9/22.
//

#ifndef AC_MUDUO_POLLER_T_H
#define AC_MUDUO_POLLER_T_H

#include "base/timestamp.h"
#include "event_loop_t.h"
#include <map>
#include <vector>

namespace ac_muduo {
    namespace net {
        class channel_t;

        class poller_t {
        public:
            typedef std::vector<channel_t *> channel_list_t;

            poller_t(event_loop_t *loop);

            virtual ~poller_t();

            virtual timestamp poll(int timeout_ms, channel_list_t *active_channels) = 0;

            virtual void update_channel(channel_t *channel) = 0;

            virtual void remove_channel(channel_t *channel) = 0;

            virtual bool has_channel(channel_t *channel);

            static poller_t *new_default_poller(event_loop_t *loop);

            void assert_in_loop_thread() const {
                this->ower_loop_->assert_in_loop_thread();
            }


        protected:
            typedef std::map<int, channel_t *> channel_map_t;
            channel_map_t channels_;

        private:
            event_loop_t *ower_loop_;
        };

    }
}


#endif //AC_MUDUO_POLLER_T_H
