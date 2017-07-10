add_executable(epic   epic.c build_pyr.c      utilities.c parse_args.c fileio.c quantize.c run_length.c huffman.c convolve.c edges.c)
add_executable(unepic unepic.c collapse_pyr.c utilities.c parse_args.c fileio.c quantize.c run_length.c huffman.c)

target_link_libraries(epic m)
