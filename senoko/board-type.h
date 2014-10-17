#ifndef __SENOKO_BOARD_TYPE_H__
#define __SENOKO_BOARD_TYPE_H__

enum board_type {
  unknown,
  senoko_full,
  senoko_half,
};

enum board_type boardType(void);

#endif /* __SENOKO_BOARD_TYPE_H__ */
