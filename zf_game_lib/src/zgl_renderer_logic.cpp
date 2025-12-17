s_gfx_resource &PushGFXResource(const e_gfx_resource_type type, s_gfx_resource_arena &arena) {
    ZF_ASSERT(type != ek_gfx_resource_type_invalid);

    s_gfx_resource &resource = Alloc<s_gfx_resource>(*arena.mem_arena);

    if (!arena.head) {
        arena.head = &resource;
        arena.tail = &resource;
    } else {
        arena.tail->next = &resource;
        arena.tail = &resource;
    }

    resource.type = type;

    return resource;
}

void s_render_instr_seq::Submit(const s_render_instr instr) {
    if (!blocks_head) {
        blocks_head = &Alloc<s_render_instr_block>(*blocks_mem_arena);
        blocks_tail = blocks_head;
    } else if (blocks_tail->instrs.IsFull()) {
        blocks_tail->next = &Alloc<s_render_instr_block>(*blocks_mem_arena);
        blocks_tail = blocks_tail->next;
    }

    blocks_tail->instrs.Append(instr);
}
