BUILD_DIR=build
T3D_INST=$(shell realpath ../..)

include $(N64_INST)/include/n64.mk
include $(N64_INST)/include//t3d.mk

N64_CFLAGS += -std=gnu2x

PROJECT_NAME=brew_christmas

src = main.c

assets_png = $(wildcard assets/*.png)
assets_gltf = $(wildcard assets/*.glb)
assets_xm = $(wildcard assets/*.xm)
assets_mp3 = $(wildcard assets/*.mp3)
assets_conv = $(addprefix filesystem/,$(notdir $(assets_png:%.png=%.sprite))) \
			  $(addprefix filesystem/,$(notdir $(assets_gltf:%.glb=%.t3dm))) \
			  $(addprefix filesystem/,$(notdir $(assets_xm:%.xm=%.xm64))) \
			  $(addprefix filesystem/,$(notdir $(assets_mp3:%.mp3=%.wav64)))

MKSPRITE_FLAGS += --mipmap BOX
filesystem/carpet1.ihq.sprite: MKSPRITE_FLAGS += --mipmap BOX

all: $(PROJECT_NAME).z64

filesystem/%.sprite: assets/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	$(N64_MKSPRITE) $(MKSPRITE_FLAGS) -o filesystem "$<"

filesystem/%.t3dm: assets/%.glb
	@mkdir -p $(dir $@)
	@echo "    [T3D-MODEL] $@"
	$(T3D_GLTF_TO_3D) "$<" $@
	$(N64_BINDIR)/mkasset -c 2 -o filesystem $@

filesystem/%.xm64: assets/%.xm
	@mkdir -p $(dir $@)
	@echo "    [XM] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o $(dir $@) "$<"

filesystem/%.wav64: assets/%.mp3
	@mkdir -p $(dir $@)
	@echo "    [SFX] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o $(dir $@) "$<"

$(BUILD_DIR)/$(PROJECT_NAME).dfs: $(assets_conv)
$(BUILD_DIR)/$(PROJECT_NAME).elf: $(src:%.c=$(BUILD_DIR)/%.o)

$(PROJECT_NAME).z64: N64_ROM_TITLE="Tiny3D - Light"
$(PROJECT_NAME).z64: $(BUILD_DIR)/$(PROJECT_NAME).dfs

clean:
	rm -rf $(BUILD_DIR) *.z64
	rm -rf filesystem

build_lib:
	rm -rf $(BUILD_DIR) *.z64
	make -C $(T3D_INST)
	make all

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
