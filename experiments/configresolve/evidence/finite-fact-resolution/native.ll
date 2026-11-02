; Flowcore target artifact: Demo
; Flowcore generic structured lowering plan: ordered blocks, calls, branches and returns
target triple = "x86_64-pc-linux-gnu"
%flow.record.2 = type { i32, i32, i32, i32 }
declare void @llvm.trap()
define i32 @flow.function.8(%flow.record.2 %flow_arg_9, %flow.record.2 %flow_arg_10, %flow.record.2 %flow_arg_11, i32 %flow_arg_12) {
entry:
  %flow_slot_9 = alloca %flow.record.2, align 8
  %flow_slot_10 = alloca %flow.record.2, align 8
  %flow_slot_11 = alloca %flow.record.2, align 8
  %flow_slot_12 = alloca i32, align 4
  %flow_slot_14 = alloca %flow.record.2, align 8
  %flow_slot_15 = alloca %flow.record.2, align 8
  %flow_slot_16 = alloca %flow.record.2, align 8
  %flow_slot_17 = alloca i32, align 4
  store %flow.record.2 %flow_arg_9, ptr %flow_slot_9
  store %flow.record.2 %flow_arg_10, ptr %flow_slot_10
  store %flow.record.2 %flow_arg_11, ptr %flow_slot_11
  store i32 %flow_arg_12, ptr %flow_slot_12
  br label %flow_block_0
flow_block_0:
  %flow_load_0 = load %flow.record.2, ptr %flow_slot_9
  %flow_record_extract_1 = extractvalue %flow.record.2 %flow_load_0, 0
  %flow_condition_2 = icmp eq i32 %flow_record_extract_1, 0
  br i1 %flow_condition_2, label %flow_block_1, label %flow_join_0
flow_block_1:
  %flow_load_3 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_4 = extractvalue %flow.record.2 %flow_load_3, 0
  %flow_condition_5 = icmp eq i32 %flow_record_extract_4, 0
  br i1 %flow_condition_5, label %flow_block_2, label %flow_join_1
flow_block_2:
  %flow_load_6 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_7 = extractvalue %flow.record.2 %flow_load_6, 0
  %flow_condition_8 = icmp eq i32 %flow_record_extract_7, 0
  br i1 %flow_condition_8, label %flow_block_3, label %flow_join_2
flow_block_3:
  ret i32 1
flow_join_2:
  %flow_load_9 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_10 = extractvalue %flow.record.2 %flow_load_9, 1
  ret i32 %flow_record_extract_10
flow_join_1:
  %flow_load_11 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_12 = extractvalue %flow.record.2 %flow_load_11, 0
  %flow_condition_13 = icmp eq i32 %flow_record_extract_12, 0
  br i1 %flow_condition_13, label %flow_block_4, label %flow_join_3
flow_block_4:
  %flow_load_14 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_15 = extractvalue %flow.record.2 %flow_load_14, 1
  ret i32 %flow_record_extract_15
flow_join_3:
  %flow_load_16 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_17 = extractvalue %flow.record.2 %flow_load_16, 2
  %flow_load_18 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_19 = extractvalue %flow.record.2 %flow_load_18, 2
  %flow_condition_20 = icmp sgt i32 %flow_record_extract_17, %flow_record_extract_19
  br i1 %flow_condition_20, label %flow_block_5, label %flow_join_4
flow_block_5:
  %flow_load_21 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_22 = extractvalue %flow.record.2 %flow_load_21, 1
  ret i32 %flow_record_extract_22
flow_join_4:
  %flow_load_23 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_24 = extractvalue %flow.record.2 %flow_load_23, 2
  %flow_load_25 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_26 = extractvalue %flow.record.2 %flow_load_25, 2
  %flow_condition_27 = icmp sgt i32 %flow_record_extract_24, %flow_record_extract_26
  br i1 %flow_condition_27, label %flow_block_6, label %flow_join_5
flow_block_6:
  %flow_load_28 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_29 = extractvalue %flow.record.2 %flow_load_28, 1
  ret i32 %flow_record_extract_29
flow_join_5:
  %flow_load_30 = load i32, ptr %flow_slot_12
  %flow_condition_31 = icmp ne i32 %flow_load_30, 0
  br i1 %flow_condition_31, label %flow_block_7, label %flow_join_6
flow_block_7:
  ret i32 3
flow_join_6:
  %flow_load_32 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_33 = extractvalue %flow.record.2 %flow_load_32, 3
  %flow_load_34 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_35 = extractvalue %flow.record.2 %flow_load_34, 3
  %flow_condition_36 = icmp sgt i32 %flow_record_extract_33, %flow_record_extract_35
  br i1 %flow_condition_36, label %flow_block_8, label %flow_join_7
flow_block_8:
  %flow_load_37 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_38 = extractvalue %flow.record.2 %flow_load_37, 1
  ret i32 %flow_record_extract_38
flow_join_7:
  %flow_load_39 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_40 = extractvalue %flow.record.2 %flow_load_39, 1
  ret i32 %flow_record_extract_40
flow_join_0:
  %flow_load_41 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_42 = extractvalue %flow.record.2 %flow_load_41, 0
  %flow_condition_43 = icmp eq i32 %flow_record_extract_42, 0
  br i1 %flow_condition_43, label %flow_block_9, label %flow_join_8
flow_block_9:
  %flow_load_44 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_45 = extractvalue %flow.record.2 %flow_load_44, 0
  %flow_condition_46 = icmp eq i32 %flow_record_extract_45, 0
  br i1 %flow_condition_46, label %flow_block_10, label %flow_join_9
flow_block_10:
  %flow_load_47 = load %flow.record.2, ptr %flow_slot_9
  %flow_record_extract_48 = extractvalue %flow.record.2 %flow_load_47, 1
  ret i32 %flow_record_extract_48
flow_join_9:
  %flow_load_49 = load %flow.record.2, ptr %flow_slot_9
  %flow_record_extract_50 = extractvalue %flow.record.2 %flow_load_49, 2
  %flow_load_51 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_52 = extractvalue %flow.record.2 %flow_load_51, 2
  %flow_condition_53 = icmp sgt i32 %flow_record_extract_50, %flow_record_extract_52
  br i1 %flow_condition_53, label %flow_block_11, label %flow_join_10
flow_block_11:
  %flow_load_54 = load %flow.record.2, ptr %flow_slot_9
  %flow_record_extract_55 = extractvalue %flow.record.2 %flow_load_54, 1
  ret i32 %flow_record_extract_55
flow_join_10:
  %flow_load_56 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_57 = extractvalue %flow.record.2 %flow_load_56, 2
  %flow_load_58 = load %flow.record.2, ptr %flow_slot_9
  %flow_record_extract_59 = extractvalue %flow.record.2 %flow_load_58, 2
  %flow_condition_60 = icmp sgt i32 %flow_record_extract_57, %flow_record_extract_59
  br i1 %flow_condition_60, label %flow_block_12, label %flow_join_11
flow_block_12:
  %flow_load_61 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_62 = extractvalue %flow.record.2 %flow_load_61, 1
  ret i32 %flow_record_extract_62
flow_join_11:
  %flow_load_63 = load i32, ptr %flow_slot_12
  %flow_condition_64 = icmp ne i32 %flow_load_63, 0
  br i1 %flow_condition_64, label %flow_block_13, label %flow_join_12
flow_block_13:
  ret i32 3
flow_join_12:
  %flow_load_65 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_66 = extractvalue %flow.record.2 %flow_load_65, 3
  %flow_load_67 = load %flow.record.2, ptr %flow_slot_9
  %flow_record_extract_68 = extractvalue %flow.record.2 %flow_load_67, 3
  %flow_condition_69 = icmp sgt i32 %flow_record_extract_66, %flow_record_extract_68
  br i1 %flow_condition_69, label %flow_block_14, label %flow_join_13
flow_block_14:
  %flow_load_70 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_71 = extractvalue %flow.record.2 %flow_load_70, 1
  ret i32 %flow_record_extract_71
flow_join_13:
  %flow_load_72 = load %flow.record.2, ptr %flow_slot_9
  %flow_record_extract_73 = extractvalue %flow.record.2 %flow_load_72, 1
  ret i32 %flow_record_extract_73
flow_join_8:
  %flow_load_74 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_75 = extractvalue %flow.record.2 %flow_load_74, 0
  %flow_condition_76 = icmp eq i32 %flow_record_extract_75, 0
  br i1 %flow_condition_76, label %flow_block_15, label %flow_join_14
flow_block_15:
  %flow_load_77 = load %flow.record.2, ptr %flow_slot_9
  %flow_record_extract_78 = extractvalue %flow.record.2 %flow_load_77, 2
  %flow_load_79 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_80 = extractvalue %flow.record.2 %flow_load_79, 2
  %flow_condition_81 = icmp sgt i32 %flow_record_extract_78, %flow_record_extract_80
  br i1 %flow_condition_81, label %flow_block_16, label %flow_join_15
flow_block_16:
  %flow_load_82 = load %flow.record.2, ptr %flow_slot_9
  %flow_record_extract_83 = extractvalue %flow.record.2 %flow_load_82, 1
  ret i32 %flow_record_extract_83
flow_join_15:
  %flow_load_84 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_85 = extractvalue %flow.record.2 %flow_load_84, 2
  %flow_load_86 = load %flow.record.2, ptr %flow_slot_9
  %flow_record_extract_87 = extractvalue %flow.record.2 %flow_load_86, 2
  %flow_condition_88 = icmp sgt i32 %flow_record_extract_85, %flow_record_extract_87
  br i1 %flow_condition_88, label %flow_block_17, label %flow_join_16
flow_block_17:
  %flow_load_89 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_90 = extractvalue %flow.record.2 %flow_load_89, 1
  ret i32 %flow_record_extract_90
flow_join_16:
  %flow_load_91 = load i32, ptr %flow_slot_12
  %flow_condition_92 = icmp ne i32 %flow_load_91, 0
  br i1 %flow_condition_92, label %flow_block_18, label %flow_join_17
flow_block_18:
  ret i32 3
flow_join_17:
  %flow_load_93 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_94 = extractvalue %flow.record.2 %flow_load_93, 3
  %flow_load_95 = load %flow.record.2, ptr %flow_slot_9
  %flow_record_extract_96 = extractvalue %flow.record.2 %flow_load_95, 3
  %flow_condition_97 = icmp sgt i32 %flow_record_extract_94, %flow_record_extract_96
  br i1 %flow_condition_97, label %flow_block_19, label %flow_join_18
flow_block_19:
  %flow_load_98 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_99 = extractvalue %flow.record.2 %flow_load_98, 1
  ret i32 %flow_record_extract_99
flow_join_18:
  %flow_load_100 = load %flow.record.2, ptr %flow_slot_9
  %flow_record_extract_101 = extractvalue %flow.record.2 %flow_load_100, 1
  ret i32 %flow_record_extract_101
flow_join_14:
  %flow_load_102 = load %flow.record.2, ptr %flow_slot_9
  %flow_record_extract_103 = extractvalue %flow.record.2 %flow_load_102, 2
  %flow_load_104 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_105 = extractvalue %flow.record.2 %flow_load_104, 2
  %flow_condition_106 = icmp sgt i32 %flow_record_extract_103, %flow_record_extract_105
  br i1 %flow_condition_106, label %flow_block_20, label %flow_block_22
flow_block_20:
  %flow_load_107 = load %flow.record.2, ptr %flow_slot_9
  %flow_record_extract_108 = extractvalue %flow.record.2 %flow_load_107, 2
  %flow_load_109 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_110 = extractvalue %flow.record.2 %flow_load_109, 2
  %flow_condition_111 = icmp sgt i32 %flow_record_extract_108, %flow_record_extract_110
  br i1 %flow_condition_111, label %flow_block_21, label %flow_join_20
flow_block_21:
  %flow_load_112 = load %flow.record.2, ptr %flow_slot_9
  %flow_record_extract_113 = extractvalue %flow.record.2 %flow_load_112, 1
  ret i32 %flow_record_extract_113
flow_join_20:
  br label %flow_join_19
flow_block_22:
  %flow_load_114 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_115 = extractvalue %flow.record.2 %flow_load_114, 2
  %flow_load_116 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_117 = extractvalue %flow.record.2 %flow_load_116, 2
  %flow_condition_118 = icmp sgt i32 %flow_record_extract_115, %flow_record_extract_117
  br i1 %flow_condition_118, label %flow_block_23, label %flow_join_21
flow_block_23:
  %flow_load_119 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_120 = extractvalue %flow.record.2 %flow_load_119, 1
  ret i32 %flow_record_extract_120
flow_join_21:
  br label %flow_join_19
flow_join_19:
  %flow_load_121 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_122 = extractvalue %flow.record.2 %flow_load_121, 2
  %flow_load_123 = load %flow.record.2, ptr %flow_slot_9
  %flow_record_extract_124 = extractvalue %flow.record.2 %flow_load_123, 2
  %flow_condition_125 = icmp sgt i32 %flow_record_extract_122, %flow_record_extract_124
  br i1 %flow_condition_125, label %flow_block_24, label %flow_join_22
flow_block_24:
  %flow_load_126 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_127 = extractvalue %flow.record.2 %flow_load_126, 2
  %flow_load_128 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_129 = extractvalue %flow.record.2 %flow_load_128, 2
  %flow_condition_130 = icmp sgt i32 %flow_record_extract_127, %flow_record_extract_129
  br i1 %flow_condition_130, label %flow_block_25, label %flow_join_23
flow_block_25:
  %flow_load_131 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_132 = extractvalue %flow.record.2 %flow_load_131, 1
  ret i32 %flow_record_extract_132
flow_join_23:
  br label %flow_join_22
flow_join_22:
  %flow_load_133 = load i32, ptr %flow_slot_12
  %flow_condition_134 = icmp ne i32 %flow_load_133, 0
  br i1 %flow_condition_134, label %flow_block_26, label %flow_join_24
flow_block_26:
  ret i32 3
flow_join_24:
  %flow_load_135 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_136 = extractvalue %flow.record.2 %flow_load_135, 3
  %flow_load_137 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_138 = extractvalue %flow.record.2 %flow_load_137, 3
  %flow_condition_139 = icmp sgt i32 %flow_record_extract_136, %flow_record_extract_138
  br i1 %flow_condition_139, label %flow_block_27, label %flow_join_25
flow_block_27:
  %flow_load_140 = load %flow.record.2, ptr %flow_slot_11
  %flow_record_extract_141 = extractvalue %flow.record.2 %flow_load_140, 1
  ret i32 %flow_record_extract_141
flow_join_25:
  %flow_load_142 = load %flow.record.2, ptr %flow_slot_10
  %flow_record_extract_143 = extractvalue %flow.record.2 %flow_load_142, 1
  ret i32 %flow_record_extract_143
flow_function_exit:
  ret i32 zeroinitializer
}
define i32 @main() {
entry:
  %flow_slot_9 = alloca %flow.record.2, align 8
  %flow_slot_10 = alloca %flow.record.2, align 8
  %flow_slot_11 = alloca %flow.record.2, align 8
  %flow_slot_12 = alloca i32, align 4
  %flow_slot_14 = alloca %flow.record.2, align 8
  %flow_slot_15 = alloca %flow.record.2, align 8
  %flow_slot_16 = alloca %flow.record.2, align 8
  %flow_slot_17 = alloca i32, align 4
  br label %flow_block_28
flow_block_28:
  %flow_record_insert_0 = insertvalue %flow.record.2 zeroinitializer, i32 1, 0
  %flow_record_insert_1 = insertvalue %flow.record.2 %flow_record_insert_0, i32 11, 1
  %flow_record_insert_2 = insertvalue %flow.record.2 %flow_record_insert_1, i32 10, 2
  %flow_record_insert_3 = insertvalue %flow.record.2 %flow_record_insert_2, i32 1, 3
  store %flow.record.2 %flow_record_insert_3, ptr %flow_slot_14
  %flow_record_insert_4 = insertvalue %flow.record.2 zeroinitializer, i32 1, 0
  %flow_record_insert_5 = insertvalue %flow.record.2 %flow_record_insert_4, i32 22, 1
  %flow_record_insert_6 = insertvalue %flow.record.2 %flow_record_insert_5, i32 20, 2
  %flow_record_insert_7 = insertvalue %flow.record.2 %flow_record_insert_6, i32 2, 3
  store %flow.record.2 %flow_record_insert_7, ptr %flow_slot_15
  %flow_record_insert_8 = insertvalue %flow.record.2 zeroinitializer, i32 1, 0
  %flow_record_insert_9 = insertvalue %flow.record.2 %flow_record_insert_8, i32 33, 1
  %flow_record_insert_10 = insertvalue %flow.record.2 %flow_record_insert_9, i32 20, 2
  %flow_record_insert_11 = insertvalue %flow.record.2 %flow_record_insert_10, i32 3, 3
  store %flow.record.2 %flow_record_insert_11, ptr %flow_slot_16
  %flow_load_12 = load %flow.record.2, ptr %flow_slot_14
  %flow_load_13 = load %flow.record.2, ptr %flow_slot_15
  %flow_load_14 = load %flow.record.2, ptr %flow_slot_16
  %flow_call_54 = call i32 @flow.function.8(%flow.record.2 %flow_load_12, %flow.record.2 %flow_load_13, %flow.record.2 %flow_load_14, i32 0)
  store i32 %flow_call_54, ptr %flow_slot_17
  store i32 %flow_call_54, ptr %flow_slot_17
  %flow_load_15 = load i32, ptr %flow_slot_17
  ret i32 %flow_load_15
flow_function_exit:
  ret i32 zeroinitializer
}
