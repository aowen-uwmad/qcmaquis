namespace ambient { namespace core {
template< typename FP, typename T0 >
operation::operation( FP op, T0 *arg0 ){
    this->init();
    this->operation_ptr = (void(*)())op;
    this->arg_count = 1;
    this->arguments = (void**)malloc(sizeof(void*)*this->arg_count);
    this->profiles = (p_profile**)malloc(sizeof(p_profile*)*this->arg_count);
    this->arguments[0] = (void*)arg0;
    void(operation::*ptr)(FP); ptr = &operation::prototype_template;
    this->prototype = (void(operation::*)())ptr;
}
template < typename T0 >
void operation::extract_profiles(int pin){
    this->profiles[0] = get_profile(*static_cast<T0*>(this->arguments[0]));
    if(pin >= 0) this->pin = this->profiles[pin];
    this->is_extracted = true;
}
template < typename T0 >
void operation::prototype_template(void (*)( T0& ))
{
    if(!this->is_extracted) this->extract_profiles< T0 >(-1);
    ((void (*)( T0& ))this->operation_ptr)
    ( *static_cast<T0*>(this->arguments[0]) );
}
template < typename T0 > void operation::prototype_template(void (*)( pinned T0& )) { if(!this->is_extracted) this->extract_profiles< T0 >(0); ((void (*)( pinned T0& ))this->operation_ptr) ( marked *static_cast<T0*>(this->arguments[0]) ); }

template< typename FP, typename T0 , typename T1 >
operation::operation( FP op, T0 *arg0 , T1 *arg1 ){
    this->init();
    this->operation_ptr = (void(*)())op;
    this->arg_count = 2;
    this->arguments = (void**)malloc(sizeof(void*)*this->arg_count);
    this->profiles = (p_profile**)malloc(sizeof(p_profile*)*this->arg_count);
    this->arguments[0] = (void*)arg0; this->arguments[1] = (void*)arg1;
    void(operation::*ptr)(FP); ptr = &operation::prototype_template;
    this->prototype = (void(operation::*)())ptr;
}
template < typename T0 , typename T1 >
void operation::extract_profiles(int pin){
    this->profiles[0] = get_profile(*static_cast<T0*>(this->arguments[0])); this->profiles[1] = get_profile(*static_cast<T1*>(this->arguments[1]));
    if(pin >= 0) this->pin = this->profiles[pin];
    this->is_extracted = true;
}
template < typename T0 , typename T1 >
void operation::prototype_template(void (*)( T0& , T1& ))
{
    if(!this->is_extracted) this->extract_profiles< T0 , T1 >(-1);
    ((void (*)( T0& , T1& ))this->operation_ptr)
    ( *static_cast<T0*>(this->arguments[0]) , *static_cast<T1*>(this->arguments[1]) );
}
template < typename T0 , typename T1 > void operation::prototype_template(void (*)( pinned T0& , T1& )) { if(!this->is_extracted) this->extract_profiles< T0 , T1 >(0); ((void (*)( pinned T0& , T1& ))this->operation_ptr) ( marked *static_cast<T0*>(this->arguments[0]) , *static_cast<T1*>(this->arguments[1]) ); } template < typename T0 , typename T1 > void operation::prototype_template(void (*)( T0& , pinned T1& )) { if(!this->is_extracted) this->extract_profiles< T0 , T1 >(1); ((void (*)( T0& , pinned T1& ))this->operation_ptr) ( *static_cast<T0*>(this->arguments[0]) , marked *static_cast<T1*>(this->arguments[1]) ); }

template< typename FP, typename T0 , typename T1 , typename T2 >
operation::operation( FP op, T0 *arg0 , T1 *arg1 , T2 *arg2 ){
    this->init();
    this->operation_ptr = (void(*)())op;
    this->arg_count = 3;
    this->arguments = (void**)malloc(sizeof(void*)*this->arg_count);
    this->profiles = (p_profile**)malloc(sizeof(p_profile*)*this->arg_count);
    this->arguments[0] = (void*)arg0; this->arguments[1] = (void*)arg1; this->arguments[2] = (void*)arg2;
    void(operation::*ptr)(FP); ptr = &operation::prototype_template;
    this->prototype = (void(operation::*)())ptr;
}
template < typename T0 , typename T1 , typename T2 >
void operation::extract_profiles(int pin){
    this->profiles[0] = get_profile(*static_cast<T0*>(this->arguments[0])); this->profiles[1] = get_profile(*static_cast<T1*>(this->arguments[1])); this->profiles[2] = get_profile(*static_cast<T2*>(this->arguments[2]));
    if(pin >= 0) this->pin = this->profiles[pin];
    this->is_extracted = true;
}
template < typename T0 , typename T1 , typename T2 >
void operation::prototype_template(void (*)( T0& , T1& , T2& ))
{
    if(!this->is_extracted) this->extract_profiles< T0 , T1 , T2 >(-1);
    ((void (*)( T0& , T1& , T2& ))this->operation_ptr)
    ( *static_cast<T0*>(this->arguments[0]) , *static_cast<T1*>(this->arguments[1]) , *static_cast<T2*>(this->arguments[2]) );
}
template < typename T0 , typename T1 , typename T2 > void operation::prototype_template(void (*)( pinned T0& , T1& , T2& )) { if(!this->is_extracted) this->extract_profiles< T0 , T1 , T2 >(0); ((void (*)( pinned T0& , T1& , T2& ))this->operation_ptr) ( marked *static_cast<T0*>(this->arguments[0]) , *static_cast<T1*>(this->arguments[1]) , *static_cast<T2*>(this->arguments[2]) ); } template < typename T0 , typename T1 , typename T2 > void operation::prototype_template(void (*)( T0& , pinned T1& , T2& )) { if(!this->is_extracted) this->extract_profiles< T0 , T1 , T2 >(1); ((void (*)( T0& , pinned T1& , T2& ))this->operation_ptr) ( *static_cast<T0*>(this->arguments[0]) , marked *static_cast<T1*>(this->arguments[1]) , *static_cast<T2*>(this->arguments[2]) ); } template < typename T0 , typename T1 , typename T2 > void operation::prototype_template(void (*)( T0& , T1& , pinned T2& )) { if(!this->is_extracted) this->extract_profiles< T0 , T1 , T2 >(2); ((void (*)( T0& , T1& , pinned T2& ))this->operation_ptr) ( *static_cast<T0*>(this->arguments[0]) , *static_cast<T1*>(this->arguments[1]) , marked *static_cast<T2*>(this->arguments[2]) ); }

template< typename FP, typename T0 , typename T1 , typename T2 , typename T3 >
operation::operation( FP op, T0 *arg0 , T1 *arg1 , T2 *arg2 , T3 *arg3 ){
    this->init();
    this->operation_ptr = (void(*)())op;
    this->arg_count = 4;
    this->arguments = (void**)malloc(sizeof(void*)*this->arg_count);
    this->profiles = (p_profile**)malloc(sizeof(p_profile*)*this->arg_count);
    this->arguments[0] = (void*)arg0; this->arguments[1] = (void*)arg1; this->arguments[2] = (void*)arg2; this->arguments[3] = (void*)arg3;
    void(operation::*ptr)(FP); ptr = &operation::prototype_template;
    this->prototype = (void(operation::*)())ptr;
}
template < typename T0 , typename T1 , typename T2 , typename T3 >
void operation::extract_profiles(int pin){
    this->profiles[0] = get_profile(*static_cast<T0*>(this->arguments[0])); this->profiles[1] = get_profile(*static_cast<T1*>(this->arguments[1])); this->profiles[2] = get_profile(*static_cast<T2*>(this->arguments[2])); this->profiles[3] = get_profile(*static_cast<T3*>(this->arguments[3]));
    if(pin >= 0) this->pin = this->profiles[pin];
    this->is_extracted = true;
}
template < typename T0 , typename T1 , typename T2 , typename T3 >
void operation::prototype_template(void (*)( T0& , T1& , T2& , T3& ))
{
    if(!this->is_extracted) this->extract_profiles< T0 , T1 , T2 , T3 >(-1);
    ((void (*)( T0& , T1& , T2& , T3& ))this->operation_ptr)
    ( *static_cast<T0*>(this->arguments[0]) , *static_cast<T1*>(this->arguments[1]) , *static_cast<T2*>(this->arguments[2]) , *static_cast<T3*>(this->arguments[3]) );
}
template < typename T0 , typename T1 , typename T2 , typename T3 > void operation::prototype_template(void (*)( pinned T0& , T1& , T2& , T3& )) { if(!this->is_extracted) this->extract_profiles< T0 , T1 , T2 , T3 >(0); ((void (*)( pinned T0& , T1& , T2& , T3& ))this->operation_ptr) ( marked *static_cast<T0*>(this->arguments[0]) , *static_cast<T1*>(this->arguments[1]) , *static_cast<T2*>(this->arguments[2]) , *static_cast<T3*>(this->arguments[3]) ); } template < typename T0 , typename T1 , typename T2 , typename T3 > void operation::prototype_template(void (*)( T0& , pinned T1& , T2& , T3& )) { if(!this->is_extracted) this->extract_profiles< T0 , T1 , T2 , T3 >(1); ((void (*)( T0& , pinned T1& , T2& , T3& ))this->operation_ptr) ( *static_cast<T0*>(this->arguments[0]) , marked *static_cast<T1*>(this->arguments[1]) , *static_cast<T2*>(this->arguments[2]) , *static_cast<T3*>(this->arguments[3]) ); } template < typename T0 , typename T1 , typename T2 , typename T3 > void operation::prototype_template(void (*)( T0& , T1& , pinned T2& , T3& )) { if(!this->is_extracted) this->extract_profiles< T0 , T1 , T2 , T3 >(2); ((void (*)( T0& , T1& , pinned T2& , T3& ))this->operation_ptr) ( *static_cast<T0*>(this->arguments[0]) , *static_cast<T1*>(this->arguments[1]) , marked *static_cast<T2*>(this->arguments[2]) , *static_cast<T3*>(this->arguments[3]) ); } template < typename T0 , typename T1 , typename T2 , typename T3 > void operation::prototype_template(void (*)( T0& , T1& , T2& , pinned T3& )) { if(!this->is_extracted) this->extract_profiles< T0 , T1 , T2 , T3 >(3); ((void (*)( T0& , T1& , T2& , pinned T3& ))this->operation_ptr) ( *static_cast<T0*>(this->arguments[0]) , *static_cast<T1*>(this->arguments[1]) , *static_cast<T2*>(this->arguments[2]) , marked *static_cast<T3*>(this->arguments[3]) ); }
}}
