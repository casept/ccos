SHELL := bash
.SHELLFLAGS := -eu -o pipefail -c
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules

.PHONY: clean run

.DEFAULT_GOAL := cccore

.PHONY: ccboot cccore

ccboot:
	$(MAKE) -C ccboot

cccore:
	$(MAKE) -C cccore

clean:
	$(MAKE) -C ccboot clean_recursive
	$(MAKE) -C cccore clean_recursive

run: ccboot cccore
	$(MAKE) -C ccboot run
