include prattle.mak

# copy the source so build, #include, etc. works basically identically
$(PRATTLE_IMPORTS): %: prattle/%
	$(info [import] $< --> $@)
	@mkdir -p src/prattle
	@cp $< $@
