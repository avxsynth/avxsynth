# Everytime this file gets changed, the same change needs to be made in common.mk.template as well
AVXSYNTH_DEPLOY_ROOT_REL_PATH   = .AVXSynth
AVXSYNTH_DEPLOY_ROOT 		= $(HOME)/$(AVXSYNTH_DEPLOY_ROOT_REL_PATH)/

AVXSYNTH_DEPLOY_REL_PATH        = .AVXSynth/plugins
AVXSYNTH_DEPLOY_PLUGINS		= $(HOME)/$(AVXSYNTH_DEPLOY_REL_PATH)/

LOG4CPP_LIBS	 = `log4cpp-config --libs`

AVXCOMMON_LIB                   = -L$(AVXSYNTH_DEPLOY_ROOT) -lavxcommon
# for maple avxsynth compilation (do not delete this flag, it is used in ffms2avs plugin)
AVXSYNTH_INCLUDE_PATH           = .
