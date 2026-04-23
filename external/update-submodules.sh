#!/bin/bash

GREEN=$(tput setaf 10)
YELLOW=$(tput setaf 11)
NONE=$(tput sgr0)


update_submodule()
{
    echo "${GREEN}${1}${NONE}: '${YELLOW}git -C $1 submodule init${NONE}'"
    git -C $1 submodule init || exit 1
    echo "${GREEN}${1}${NONE}: '${YELLOW}git -C $1 submodule update --recursive${NONE}'"
    git -C $1 submodule update --recursive || exit 2
}


update_submodule libcurlwrapper
update_submodule libwupsxx

exit 0
