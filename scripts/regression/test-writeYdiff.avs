DGSource("DirtyMary.dgx")
ConvertToYV12()
WriteFile("DirtyMary.ydf", "current_frame", """ " " """, "YDifferenceFromPrevious")
