COLOR_RED = \033[31m
COLOR_GREEN = \033[32m
COLOR_YELLOW = \033[33m
COLOR_BLUE = \033[34m
COLOR_RESET = \033[0m

define text_green
$(COLOR_GREEN)$(1)$(COLOR_RESET)
endef

define text_blue
$(COLOR_BLUE)$(1)$(COLOR_RESET)
endef

define text_yellow
$(COLOR_YELLOW)$(1)$(COLOR_RESET)
endef

define echo_tag_green
printf "[$(call text_green,$(1))]$(2)\n"
endef

define echo_tag_blue
printf "[$(call text_blue,$(1))]$(2)"
endef

define echo_tag_yellow
printf "[$(call text_yellow,$(1))]$(2)\n"
endef