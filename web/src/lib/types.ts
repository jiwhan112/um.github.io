import { z } from 'zod';

export type Inline = { text: string; kind?: 'text'|'strong'|'em'|'code'|'link'|'br'; href?: string };
export type Block =
 | { type: 'paragraph'|'heading'; content: Inline[]; level?: number; id?: string }
 | { type: 'code'; text: string; language: string }
 | { type: 'list'; ordered: boolean; items: Block[][] }
 | { type: 'table'; rows: {header: boolean; cells: Inline[][]}[] }
 | { type: 'details'; title: string; blocks: Block[] }
 | { type: 'image'; src: string; alt: string; caption?: string }
 | { type: 'group'|'callout'|'grid'; blocks: Block[]; title?: string }
 | { type: 'diagram'; items: string[] };
export const inlineSchema: z.ZodType<Inline> = z.object({text:z.string(),kind:z.enum(['text','strong','em','code','link','br']).optional(),href:z.string().optional()});
export const blockSchema: z.ZodType<Block> = z.lazy(()=>z.union([
 z.object({type:z.enum(['paragraph','heading']),content:z.array(inlineSchema),level:z.number().optional(),id:z.string().optional()}),
 z.object({type:z.literal('code'),text:z.string(),language:z.string()}),
 z.object({type:z.literal('list'),ordered:z.boolean(),items:z.array(z.array(blockSchema))}),
 z.object({type:z.literal('table'),rows:z.array(z.object({header:z.boolean(),cells:z.array(z.array(inlineSchema))}))}),
 z.object({type:z.literal('details'),title:z.string(),blocks:z.array(blockSchema)}),
 z.object({type:z.literal('image'),src:z.string(),alt:z.string(),caption:z.string().optional()}),
 z.object({type:z.enum(['group','callout','grid']),blocks:z.array(blockSchema),title:z.string().optional()}),
 z.object({type:z.literal('diagram'),items:z.array(z.string())})
]));
export const documentSchema = z.object({
 id:z.string(),route:z.string(),kind:z.enum(['thinking','opensource','game-design','asset','report','knowledge']),title:z.string(),subtitle:z.string(),date:z.string(),number:z.string().optional(),videoUrl:z.string().url().optional(),videoId:z.string().optional(),
 tags:z.array(z.string()),category:z.string().default(''),topic:z.string().default(''),memoKey:z.string().optional(),starter:z.string().optional(),
 sections:z.array(z.object({id:z.string(),title:z.string(),tab:z.string(),blocks:z.array(blockSchema)})),
 source:z.string(),sourceHash:z.string()
});
export type Document = z.infer<typeof documentSchema>;
export type DocumentMeta = Omit<Document,'sections'|'starter'> & { file:string; sections:{id:string;title:string;tab:string}[] };
export const exampleSchema=z.object({en:z.string().optional(),jp:z.string().optional(),pron:z.string().default(''),ko:z.string().default('')});
const expressionSchema=z.object({text:z.string(),meaning:z.string()});
export const lessonSchema=z.object({
 id:z.number().int().positive(),date:z.string(),title:z.string(),conversation_level:z.string().optional(),reading_level:z.string().optional(),grammar_level:z.string().optional(),source_work:z.string().optional(),
 conversation:z.array(exampleSchema).default([]),reading:z.array(exampleSchema).default([]),words:z.array(exampleSchema).default([]),examples:z.array(exampleSchema).default([]),
 expressions:z.array(expressionSchema).default([]),speaking_points:z.array(z.object({title:z.string(),explanation:z.string()})).default([]),
 article:z.object({title:z.string(),publisher:z.string(),date:z.string(),url:z.string()}).optional(),
 vocabulary:z.array(z.object({term:z.string(),meaning:z.string()})).default([]),useful_expressions:z.array(expressionSchema).default([]),
 reading_check:z.array(z.object({q:z.string(),a:z.string()})).default([]),
 grammar:z.object({title:z.string(),explanation:z.string()}).optional(),
 kana:z.array(z.union([z.string(),z.object({jp:z.string(),pron:z.string().optional(),romaji:z.string().optional()})])).default([]),
 review_reuse:z.array(z.object({text:z.string().optional(),term:z.string().optional(),meaning:z.string().optional(),from_lesson:z.number().optional()})).default([])
}).passthrough();
export type Lesson=z.infer<typeof lessonSchema>;
export type Example=z.infer<typeof exampleSchema>;
export type Language='english'|'japanese';
export type LessonMeta={id:number;date:string;title:string;file:string;sections?:string[]};
export const articleSchema=z.object({id:z.string(),date:z.string(),title:z.string(),category:z.string(),source:z.string(),published:z.string(),summary:z.string(),easy:z.string(),technical:z.string(),developer:z.string(),importance:z.string(),depth:z.enum(['deep','brief']).optional(),tags:z.array(z.string()).optional(),short:z.array(z.string()),links:z.array(z.object({label:z.string(),url:z.string()})),blocks:z.array(blockSchema)});
export type Article=z.infer<typeof articleSchema>;
export type AssetMeta={id:string;name:string;src:string;route:string;experiment:string;date:string;status:string;size:number[];model:string};
export type ReviewItem={language:Language;from_lesson:number;term:string;due_date:string;status:string};
export type Catalog={version:2;revision:string;documents:DocumentMeta[];languages:Record<Language,LessonMeta[]>;assets:AssetMeta[];dates:string[];articleCount:number;review:ReviewItem[]};
export const catalogSchema=z.object({version:z.literal(2),revision:z.string(),documents:z.array(z.object({id:z.string(),route:z.string(),file:z.string(),kind:z.enum(['thinking','opensource','game-design','asset','report','knowledge']),title:z.string(),subtitle:z.string(),date:z.string(),videoUrl:z.string().url().optional(),videoId:z.string().optional(),tags:z.array(z.string()),category:z.string().default(''),topic:z.string().default(''),sections:z.array(z.object({id:z.string(),title:z.string(),tab:z.string()}))}).passthrough()),languages:z.object({english:z.array(z.object({id:z.number(),date:z.string(),title:z.string(),file:z.string(),sections:z.array(z.string()).optional()})),japanese:z.array(z.object({id:z.number(),date:z.string(),title:z.string(),file:z.string(),sections:z.array(z.string()).optional()}))}),assets:z.array(z.object({id:z.string(),name:z.string(),src:z.string(),route:z.string(),experiment:z.string(),date:z.string(),status:z.string(),size:z.array(z.number()),model:z.string()})),dates:z.array(z.string()),articleCount:z.number(),review:z.array(z.object({language:z.enum(['english','japanese']),from_lesson:z.number(),term:z.string(),due_date:z.string(),status:z.string()}))});
